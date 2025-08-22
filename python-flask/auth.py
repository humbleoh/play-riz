from flask import request, jsonify
from functools import wraps
import jwt
import datetime
import os

# Configuration
# In a production environment, these should be stored securely
JWT_SECRET = os.environ.get('JWT_SECRET', 'your-secret-key')  # Use environment variable in production
JWT_ALGORITHM = 'HS256'
JWT_EXPIRATION_DELTA = datetime.timedelta(minutes=5)

# Client credentials (in a real app, store these in a database)
CLIENT_CREDENTIALS = {
    'client1': 'secret1',
    'client2': 'secret2'
}

def generate_token(client_id):
    """
    Generate a JWT token for the given client_id
    """
    payload = {
        'client_id': client_id,
        'exp': datetime.datetime.utcnow() + JWT_EXPIRATION_DELTA,
        'iat': datetime.datetime.utcnow()
    }
    
    return jwt.encode(payload, JWT_SECRET, algorithm=JWT_ALGORITHM)

def authenticate_client(client_id, client_secret):
    """
    Authenticate a client using client_id and client_secret
    """
    if client_id in CLIENT_CREDENTIALS and CLIENT_CREDENTIALS[client_id] == client_secret:
        return True
    return False

def token_required(f):
    """
    Decorator to protect routes with JWT token
    """
    @wraps(f)
    def decorated(*args, **kwargs):
        token = None
        
        # Check if token is in headers
        auth_header = request.headers.get('Authorization')
        if auth_header:
            if auth_header.startswith('Bearer '):
                token = auth_header.split(' ')[1]
        
        if not token:
            return {'error': 'Token is missing'}, 401
        
        try:
            # Decode the token
            payload = jwt.decode(token, JWT_SECRET, algorithms=[JWT_ALGORITHM])
            request.client_id = payload['client_id']
        except jwt.ExpiredSignatureError:
            return {'error': 'Token has expired'}, 401
        except jwt.InvalidTokenError:
            return {'error': 'Invalid token'}, 401
            
        return f(*args, **kwargs)
    
    return decorated

def get_token_route():
    """
    OAuth 2.0 token endpoint (client credentials flow)
    """
    auth = request.authorization
    
    if not auth or not auth.username or not auth.password:
        # Check if credentials are in form data (also common in OAuth)
        client_id = request.form.get('client_id')
        client_secret = request.form.get('client_secret')
        
        if not client_id or not client_secret:
            return jsonify({'error': 'Client credentials required'}), 401
    else:
        client_id = auth.username
        client_secret = auth.password
    
    if authenticate_client(client_id, client_secret):
        token = generate_token(client_id)
        return jsonify({
            'access_token': token,
            'token_type': 'Bearer',
            'expires_in': JWT_EXPIRATION_DELTA.total_seconds()
        })
    
    return jsonify({'error': 'Invalid client credentials'}), 401