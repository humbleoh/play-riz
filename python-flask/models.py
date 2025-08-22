from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
import bcrypt

db = SQLAlchemy()

# Association table for many-to-many relationship between users and roles
user_roles = db.Table('user_roles',
    db.Column('user_id', db.Integer, db.ForeignKey('user.id'), primary_key=True),
    db.Column('role_id', db.Integer, db.ForeignKey('role.id'), primary_key=True)
)

# Association table for many-to-many relationship between roles and permissions
role_permissions = db.Table('role_permissions',
    db.Column('role_id', db.Integer, db.ForeignKey('role.id'), primary_key=True),
    db.Column('permission_id', db.Integer, db.ForeignKey('permission.id'), primary_key=True)
)

class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)
    password_hash = db.Column(db.String(128), nullable=False)
    client_id = db.Column(db.String(100), unique=True, nullable=True)  # For OAuth client credentials
    client_secret = db.Column(db.String(128), nullable=True)  # Hashed client secret
    is_active = db.Column(db.Boolean, default=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    # Many-to-many relationship with roles
    roles = db.relationship('Role', secondary=user_roles, lazy='subquery',
                           backref=db.backref('users', lazy=True))
    
    def set_password(self, password):
        """Hash and set the user's password"""
        self.password_hash = bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt()).decode('utf-8')
    
    def check_password(self, password):
        """Check if the provided password matches the user's password"""
        return bcrypt.checkpw(password.encode('utf-8'), self.password_hash.encode('utf-8'))
    
    def set_client_secret(self, client_secret):
        """Hash and set the client secret for OAuth"""
        self.client_secret = bcrypt.hashpw(client_secret.encode('utf-8'), bcrypt.gensalt()).decode('utf-8')
    
    def check_client_secret(self, client_secret):
        """Check if the provided client secret matches"""
        if not self.client_secret:
            return False
        return bcrypt.checkpw(client_secret.encode('utf-8'), self.client_secret.encode('utf-8'))
    
    def has_permission(self, permission_name):
        """Check if user has a specific permission through their roles"""
        for role in self.roles:
            for permission in role.permissions:
                if permission.name == permission_name:
                    return True
        return False
    
    def get_permissions(self):
        """Get all permissions for this user"""
        permissions = set()
        for role in self.roles:
            for permission in role.permissions:
                permissions.add(permission.name)
        return list(permissions)
    
    def to_dict(self):
        return {
            'id': self.id,
            'username': self.username,
            'email': self.email,
            'client_id': self.client_id,
            'is_active': self.is_active,
            'created_at': self.created_at.isoformat(),
            'roles': [role.name for role in self.roles],
            'permissions': self.get_permissions()
        }

class Role(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(80), unique=True, nullable=False)
    description = db.Column(db.String(255))
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    # Many-to-many relationship with permissions
    permissions = db.relationship('Permission', secondary=role_permissions, lazy='subquery',
                                 backref=db.backref('roles', lazy=True))
    
    def to_dict(self):
        return {
            'id': self.id,
            'name': self.name,
            'description': self.description,
            'created_at': self.created_at.isoformat(),
            'permissions': [permission.name for permission in self.permissions]
        }

class Permission(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(80), unique=True, nullable=False)
    description = db.Column(db.String(255))
    resource = db.Column(db.String(80))  # e.g., 'items', 'users'
    action = db.Column(db.String(80))    # e.g., 'read', 'write', 'delete'
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'name': self.name,
            'description': self.description,
            'resource': self.resource,
            'action': self.action,
            'created_at': self.created_at.isoformat()
        }

def init_default_data():
    """Initialize default roles and permissions"""
    # Create default permissions
    permissions_data = [
        {'name': 'items:read', 'description': 'Read items', 'resource': 'items', 'action': 'read'},
        {'name': 'items:write', 'description': 'Create and update items', 'resource': 'items', 'action': 'write'},
        {'name': 'items:delete', 'description': 'Delete items', 'resource': 'items', 'action': 'delete'},
        {'name': 'users:read', 'description': 'Read users', 'resource': 'users', 'action': 'read'},
        {'name': 'users:write', 'description': 'Create and update users', 'resource': 'users', 'action': 'write'},
        {'name': 'users:delete', 'description': 'Delete users', 'resource': 'users', 'action': 'delete'},
    ]
    
    for perm_data in permissions_data:
        if not Permission.query.filter_by(name=perm_data['name']).first():
            permission = Permission(**perm_data)
            db.session.add(permission)
    
    # Create default roles
    roles_data = [
        {'name': 'admin', 'description': 'Full access to all resources'},
        {'name': 'user', 'description': 'Basic user with read access to items'},
        {'name': 'manager', 'description': 'Can manage items but not users'},
    ]
    
    for role_data in roles_data:
        if not Role.query.filter_by(name=role_data['name']).first():
            role = Role(**role_data)
            db.session.add(role)
    
    db.session.commit()
    
    # Assign permissions to roles
    admin_role = Role.query.filter_by(name='admin').first()
    user_role = Role.query.filter_by(name='user').first()
    manager_role = Role.query.filter_by(name='manager').first()
    
    if admin_role:
        # Admin gets all permissions
        all_permissions = Permission.query.all()
        admin_role.permissions = all_permissions
    
    if user_role:
        # User gets only read permissions for items
        read_items = Permission.query.filter_by(name='items:read').first()
        if read_items:
            user_role.permissions = [read_items]
    
    if manager_role:
        # Manager gets all item permissions
        item_permissions = Permission.query.filter(Permission.resource == 'items').all()
        manager_role.permissions = item_permissions
    
    db.session.commit()