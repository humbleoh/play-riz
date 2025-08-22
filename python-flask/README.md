# RESTful API

A simple RESTful API built with Flask and Flask-RESTful.

## Requirements

- Python 3.7+
- UV package manager (optional)

## Installation

### Using standard Python tools

```bash
# Create a virtual environment
python -m venv venv

# Activate the virtual environment
# On macOS/Linux:
source venv/bin/activate
# On Windows:
# venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt
```

### Using UV (recommended)

```bash
uv venv
source .venv/bin/activate
uv pip install -r requirements.txt
```

## Running the API

```bash
python main.py
```

The API will be available at https://localhost:8000 (SSL enabled)

## Authentication

This API uses OAuth 2.0 Client Credentials flow for machine-to-machine (M2M) authentication with role-based access control (RBAC). All protected endpoints require a valid access token and appropriate permissions.

### User Management & Permissions

The API includes a comprehensive user management system with:
- **Users**: Individual accounts with credentials and roles
- **Roles**: Groups of permissions (admin, manager, user)
- **Permissions**: Specific access rights (items:read, items:write, items:delete, users:read, users:write, users:delete)

### Default Admin Account

A default admin account is created on first startup:
- **Username**: admin
- **Client ID**: admin_client
- **Client Secret**: admin_secret
- **Roles**: admin (full access)

**⚠️ Change these credentials in production!**

### Getting an Access Token

```bash
curl -X POST "https://localhost:8000/oauth/token" \
  -H "Content-Type: application/json" \
  -d '{"client_id": "admin_client", "client_secret": "admin_secret", "grant_type": "client_credentials"}' \
  --insecure
```

**Response includes both access and refresh tokens:**
```json
{
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "refresh_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "token_type": "Bearer",
  "expires_in": 300,
  "refresh_expires_in": 604800,
  "user": {
    "username": "admin",
    "roles": ["admin"],
    "permissions": ["items:read", "items:write", "items:delete", "users:read", "users:write", "users:delete"]
  }
}
```

**Note**: Access tokens expire after 5 minutes, refresh tokens expire after 7 days.

### Refreshing Access Tokens

When your access token expires, use the refresh token to get new tokens without re-authenticating:

```bash
curl -X POST "https://localhost:8000/oauth/refresh" \
  -H "Content-Type: application/json" \
  -d '{"refresh_token": "your_refresh_token_here", "grant_type": "refresh_token"}' \
  --insecure
```

**Response provides new access and refresh tokens:**
```json
{
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "refresh_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "token_type": "Bearer",
  "expires_in": 300,
  "refresh_expires_in": 604800,
  "user": {...}
}
```

## Available Endpoints

### Public Endpoints
- `GET /`: Welcome message (no authentication required)
- `POST /oauth/token`: Get access token for authentication
- `POST /oauth/refresh`: Refresh access token using refresh token

### Item Management (Requires Authentication + Permissions)
- `POST /items`: Create a new item (requires `items:write` permission)
- `GET /items`: List all items (requires `items:read` permission)
- `GET /items/<item_id>`: Get a specific item (requires `items:read` permission)
- `PUT /items/<item_id>`: Update an item (requires `items:write` permission)
- `DELETE /items/<item_id>`: Delete an item (requires `items:delete` permission)

### User Management (Requires Authentication + Permissions)
- `GET /users`: List all users (requires `users:read` permission)
- `GET /users/<user_id>`: Get a specific user (requires `users:read` permission)
- `POST /users`: Create a new user (requires `users:write` permission)
- `PUT /users/<user_id>`: Update a user (requires `users:write` permission)
- `DELETE /users/<user_id>`: Delete a user (requires `users:delete` permission)

### Role & Permission Management (Admin Only)
- `GET /roles`: List all roles (requires `admin` role)
- `GET /permissions`: List all permissions (requires `admin` role)

## Example Usage

### 1. Get an Access Token

```bash
curl -X POST "https://localhost:8000/oauth/token" \
  -H "Content-Type: application/json" \
  -d '{"client_id": "admin_client", "client_secret": "admin_secret", "grant_type": "client_credentials"}' \
  --insecure
```

Response:
```json
{
  "access_token": "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9...",
  "token_type": "Bearer",
  "expires_in": 300,
  "user": {
    "username": "admin",
    "roles": ["admin"],
    "permissions": ["items:read", "items:write", "items:delete", "users:read", "users:write", "users:delete"]
  }
}
```

### 2. Use the Token with API Endpoints

Replace `YOUR_ACCESS_TOKEN` with the actual token from step 1.

### Create an item

```bash
curl -X POST "https://localhost:8000/items" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -d '{"name": "Laptop", "description": "High-performance laptop", "price": 999.99, "tax": 10.0}' \
  --insecure
```

### Get all items

```bash
curl -X GET "https://localhost:8000/items" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  --insecure
```

### Get a specific item

```bash
curl -X GET "https://localhost:8000/items/1" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  --insecure
```

### Update an item

```bash
curl -X PUT "https://localhost:8000/items/1" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -d '{"name": "Updated Laptop", "description": "High-performance laptop with updates", "price": 1099.99, "tax": 10.0}' \
  --insecure
```

### Delete an item

```bash
curl -X DELETE "https://localhost:8000/items/1" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  --insecure
```

## User Management Examples

### Create a new user

```bash
curl -X POST "https://localhost:8000/users" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -d '{
    "username": "john_doe",
    "email": "john@example.com",
    "password": "secure_password123",
    "client_id": "john_client",
    "roles": ["user"]
  }' \
  --insecure
```

### Get all users

```bash
curl -X GET "https://localhost:8000/users" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  --insecure
```

### Get a specific user

```bash
curl -X GET "https://localhost:8000/users/2" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  --insecure
```

### Update a user

```bash
curl -X PUT "https://localhost:8000/users/2" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -d '{
    "username": "john_doe_updated",
    "email": "john.updated@example.com",
    "roles": ["manager"]
  }' \
  --insecure
```

### Delete a user

```bash
curl -X DELETE "https://localhost:8000/users/2" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  --insecure
```

### Get all roles (Admin only)

```bash
curl -X GET "https://localhost:8000/roles" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  --insecure
```

### Get all permissions (Admin only)

```bash
curl -X GET "https://localhost:8000/permissions" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  --insecure
```

## Permission System

### Default Roles and Permissions

- **admin**: Full access to all resources (items:*, users:*)
- **manager**: Can manage items and read users (items:*, users:read)
- **user**: Can only read items (items:read)

### Custom Permissions

You can create users with specific permission combinations by assigning appropriate roles or by extending the permission system in the code.

## Database Inspection

The application uses SQLite database stored at `instance/app.db`. You can inspect the database contents using several methods:

### Method 1: SQLite Command Line

```bash
# Open the database
sqlite3 instance/app.db

# List all tables
.tables

# Show table schema
.schema

# Query users
SELECT * FROM user;

# Query roles
SELECT * FROM role;

# Query permissions
SELECT * FROM permission;

# Query user roles relationship
SELECT u.username, r.name as role 
FROM user u 
JOIN user_roles ur ON u.id = ur.user_id 
JOIN role r ON ur.role_id = r.id;

# Query role permissions relationship
SELECT r.name as role, p.name as permission 
FROM role r 
JOIN role_permissions rp ON r.id = rp.role_id 
JOIN permission p ON rp.permission_id = p.id 
ORDER BY r.name, p.name;

# Exit SQLite
.quit
```

### Method 2: Python Database Inspector Script

A convenient Python script `db_inspector.py` is provided for easier database inspection:

```bash
# Run the database inspector
python db_inspector.py
```

This script provides:
- List of all database tables
- Users with their assigned roles
- Roles with their associated permissions
- All available permissions
- User-specific permission details

### Method 3: GUI Tools (Optional)

You can also use GUI tools like:
- **DB Browser for SQLite** (free, cross-platform)
- **SQLiteStudio** (free, cross-platform)
- **DataGrip** (JetBrains, paid)

Simply open the `instance/app.db` file in any of these tools.

## Security Notes

- Change default admin credentials in production
- Use HTTPS in production environments
- Tokens expire after 5 minutes - implement token refresh if needed
- Store client secrets securely
- Regularly audit user permissions and roles