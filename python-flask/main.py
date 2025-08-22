from flask import Flask, request, jsonify
from flask_restful import Api, Resource
import json
import uuid
from auth import token_required, get_token_route, require_permission, require_role
from models import db, User, Role, Permission, init_default_data

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///app.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

# Initialize database
db.init_app(app)
api = Api(app)

# Create tables and initialize default data
with app.app_context():
    db.create_all()
    init_default_data()
    
    # Create default admin user if it doesn't exist
    admin_user = User.query.filter_by(username='admin').first()
    if not admin_user:
        admin_user = User(
            username='admin',
            email='admin@example.com',
            client_id='admin_client',
            is_active=True
        )
        admin_user.set_password('admin123')  # Change this in production!
        admin_user.set_client_secret('admin_secret')  # Change this in production!
        
        # Assign admin role
        admin_role = Role.query.filter_by(name='admin').first()
        if admin_role:
            admin_user.roles.append(admin_role)
        
        db.session.add(admin_user)
        db.session.commit()
        print("Default admin user created: admin/admin123 (client_id: admin_client, client_secret: admin_secret)")

# Sample in-memory database
items_db = {}
item_id_counter = 0

# Item Resource
class ItemResource(Resource):
    @token_required
    @require_permission('items:read')
    def get(self, item_id=None):
        if item_id is None:
            # Return all items
            return [{"id": id, **item} for id, item in items_db.items()]
        else:
            # Return specific item
            if item_id not in items_db:
                return {"error": "Item not found"}, 404
            return {"id": item_id, **items_db[item_id]}
    
    @token_required
    @require_permission('items:write')
    def post(self):
        global item_id_counter
        data = request.get_json()
        
        # Validate required fields
        if not all(key in data for key in ["name", "price"]):
            return {"error": "Missing required fields"}, 400
        
        item_id_counter += 1
        items_db[item_id_counter] = {
            "name": data["name"],
            "description": data.get("description"),
            "price": data["price"],
            "tax": data.get("tax")
        }
        
        return {"id": item_id_counter, **items_db[item_id_counter]}, 201
    
    @token_required
    @require_permission('items:write')
    def put(self, item_id):
        if item_id not in items_db:
            return {"error": "Item not found"}, 404
            
        data = request.get_json()
        
        # Validate required fields
        if not all(key in data for key in ["name", "price"]):
            return {"error": "Missing required fields"}, 400
            
        items_db[item_id] = {
            "name": data["name"],
            "description": data.get("description"),
            "price": data["price"],
            "tax": data.get("tax")
        }
        
        return {"id": item_id, **items_db[item_id]}
    
    @token_required
    @require_permission('items:delete')
    def delete(self, item_id):
        if item_id not in items_db:
            return {"error": "Item not found"}, 404
            
        del items_db[item_id]
        return "", 204

# User Management Resource
class UserResource(Resource):
    @token_required
    @require_permission('users:read')
    def get(self, user_id=None):
        if user_id is None:
            # Return all users
            users = User.query.all()
            return [user.to_dict() for user in users]
        else:
            # Return specific user
            user = User.query.get(user_id)
            if not user:
                return {"error": "User not found"}, 404
            return user.to_dict()
    
    @token_required
    @require_permission('users:write')
    def post(self):
        data = request.get_json()
        
        # Validate required fields
        required_fields = ['username', 'email', 'password']
        if not all(key in data for key in required_fields):
            return {"error": "Missing required fields"}, 400
        
        # Check if user already exists
        if User.query.filter_by(username=data['username']).first():
            return {"error": "Username already exists"}, 400
        if User.query.filter_by(email=data['email']).first():
            return {"error": "Email already exists"}, 400
        
        # Create new user
        user = User(
            username=data['username'],
            email=data['email'],
            client_id=str(uuid.uuid4()),
            is_active=data.get('is_active', True)
        )
        user.set_password(data['password'])
        
        # Generate client secret for OAuth
        client_secret = str(uuid.uuid4())
        user.set_client_secret(client_secret)
        
        # Assign roles if provided
        if 'roles' in data:
            for role_name in data['roles']:
                role = Role.query.filter_by(name=role_name).first()
                if role:
                    user.roles.append(role)
        
        db.session.add(user)
        db.session.commit()
        
        response_data = user.to_dict()
        response_data['client_secret'] = client_secret  # Only show once during creation
        
        return response_data, 201
    
    @token_required
    @require_permission('users:write')
    def put(self, user_id):
        user = User.query.get(user_id)
        if not user:
            return {"error": "User not found"}, 404
        
        data = request.get_json()
        
        # Update user fields
        if 'username' in data:
            if User.query.filter_by(username=data['username']).filter(User.id != user_id).first():
                return {"error": "Username already exists"}, 400
            user.username = data['username']
        
        if 'email' in data:
            if User.query.filter_by(email=data['email']).filter(User.id != user_id).first():
                return {"error": "Email already exists"}, 400
            user.email = data['email']
        
        if 'password' in data:
            user.set_password(data['password'])
        
        if 'is_active' in data:
            user.is_active = data['is_active']
        
        # Update roles if provided
        if 'roles' in data:
            user.roles.clear()
            for role_name in data['roles']:
                role = Role.query.filter_by(name=role_name).first()
                if role:
                    user.roles.append(role)
        
        db.session.commit()
        return user.to_dict()
    
    @token_required
    @require_permission('users:delete')
    def delete(self, user_id):
        user = User.query.get(user_id)
        if not user:
            return {"error": "User not found"}, 404
        
        db.session.delete(user)
        db.session.commit()
        return "", 204

# Role Management Resource
class RoleResource(Resource):
    @token_required
    @require_role('admin')
    def get(self, role_id=None):
        if role_id is None:
            roles = Role.query.all()
            return [role.to_dict() for role in roles]
        else:
            role = Role.query.get(role_id)
            if not role:
                return {"error": "Role not found"}, 404
            return role.to_dict()

# Permission Management Resource
class PermissionResource(Resource):
    @token_required
    @require_role('admin')
    def get(self, permission_id=None):
        if permission_id is None:
            permissions = Permission.query.all()
            return [permission.to_dict() for permission in permissions]
        else:
            permission = Permission.query.get(permission_id)
            if not permission:
                return {"error": "Permission not found"}, 404
            return permission.to_dict()

# Register resources
api.add_resource(ItemResource, '/items', '/items/<int:item_id>')
api.add_resource(UserResource, '/users', '/users/<int:user_id>')
api.add_resource(RoleResource, '/roles', '/roles/<int:role_id>')
api.add_resource(PermissionResource, '/permissions', '/permissions/<int:permission_id>')

@app.route('/')
def home():
    return jsonify({"message": "Welcome to the UV RESTful API"})

# OAuth token endpoint
@app.route('/oauth/token', methods=['POST'])
def token():
    return get_token_route()

if __name__ == "__main__":
    # Run with SSL
    app.run(
        host="0.0.0.0", 
        port=8000, 
        debug=True,
        ssl_context=('ssl/cert.pem', 'ssl/key.pem')
    )