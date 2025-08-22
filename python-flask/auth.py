from flask import request, jsonify
from functools import wraps
import jwt
from datetime import datetime, timedelta
from models import User, db

# Configuration
JWT_SECRET = 'your-secret-key-change-in-production'
JWT_ALGORITHM = 'HS256'
JWT_EXPIRATION_DELTA = timedelta(minutes=5)

def generate_token(client_id):
    """
    Generate a JWT token for the given client_id
    """
    payload = {
        'client_id': client_id,
        'exp': datetime.utcnow() + JWT_EXPIRATION_DELTA,
        'iat': datetime.utcnow()
    }
    
    return jwt.encode(payload, JWT_SECRET, algorithm=JWT_ALGORITHM)

def authenticate_client(client_id, client_secret):
    """
    Authenticate client using client credentials from database
    """
    user = User.query.filter_by(client_id=client_id, is_active=True).first()
    if user and user.check_client_secret(client_secret):
        return user
    return None

def token_required(f):
    """
    Decorator to require a valid JWT token for accessing protected routes
    """
    @wraps(f)
    def decorated(*args, **kwargs):
        token = None
        
        # Get token from Authorization header
        if 'Authorization' in request.headers:
            auth_header = request.headers['Authorization']
            try:
                token = auth_header.split(" ")[1]  # Bearer <token>
            except IndexError:
                return {'error': 'Invalid token format'}, 401
        
        if not token:
            return {'error': 'Token is missing'}, 401
        
        try:
            # Decode the token
            payload = jwt.decode(token, JWT_SECRET, algorithms=[JWT_ALGORITHM])
            client_id = payload['client_id']
            
            # Get user from database
            user = User.query.filter_by(client_id=client_id, is_active=True).first()
            if not user:
                return {'error': 'Invalid client'}, 401
            
            # Store user in request context
            request.current_user = user
            request.client_id = client_id
            
        except jwt.ExpiredSignatureError:
            return {'error': 'Token has expired'}, 401
        except jwt.InvalidTokenError:
            return {'error': 'Invalid token'}, 401
            
        return f(*args, **kwargs)
    return decorated

def get_token_route():
    """
    OAuth 2.0 token endpoint for client credentials flow
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'error': 'No JSON data provided'}), 400
        
        client_id = data.get('client_id')
        client_secret = data.get('client_secret')
        grant_type = data.get('grant_type')
        
        # Validate required fields
        if not all([client_id, client_secret, grant_type]):
            return jsonify({'error': 'Missing required fields'}), 400
        
        # Only support client_credentials grant type
        if grant_type != 'client_credentials':
            return jsonify({'error': 'Unsupported grant type'}), 400
        
        # Authenticate client
        user = authenticate_client(client_id, client_secret)
        if not user:
            return jsonify({'error': 'Invalid client credentials'}), 401
        
        # Generate token
        token = generate_token(client_id)
        
        return jsonify({
            'access_token': token,
            'token_type': 'Bearer',
            'expires_in': int(JWT_EXPIRATION_DELTA.total_seconds()),
            'user': {
                'username': user.username,
                'roles': [role.name for role in user.roles],
                'permissions': user.get_permissions()
            }
        })
        
    except Exception as e:
        return jsonify({'error': 'Internal server error'}), 500

def require_permission(permission_name):
    """
    Decorator to require a specific permission for accessing a route
    Must be used after @token_required
    """
    def decorator(f):
        @wraps(f)
        def decorated(*args, **kwargs):
            # Check if user is authenticated (should be set by token_required)
            if not hasattr(request, 'current_user') or not request.current_user:
                return {'error': 'Authentication required'}, 401
            
            # Check if user has the required permission
            if not request.current_user.has_permission(permission_name):
                return {'error': f'Permission denied. Required: {permission_name}'}, 403
            
            return f(*args, **kwargs)
        return decorated
    return decorator

def require_role(role_name):
    """
    Decorator to require a specific role for accessing a route
    Must be used after @token_required
    """
    def decorator(f):
        @wraps(f)
        def decorated(*args, **kwargs):
            # Check if user is authenticated (should be set by token_required)
            if not hasattr(request, 'current_user') or not request.current_user:
                return {'error': 'Authentication required'}, 401
            
            # Check if user has the required role
            user_roles = [role.name for role in request.current_user.roles]
            if role_name not in user_roles:
                return {'error': f'Role required: {role_name}'}, 403
            
            return f(*args, **kwargs)
        return decorated
    return decorator