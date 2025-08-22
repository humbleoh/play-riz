#!/usr/bin/env python3
"""
Database Inspector for Flask User Management System
Usage: python3 db_inspector.py
"""

import sqlite3
import sys
from datetime import datetime

def connect_db():
    """Connect to the SQLite database"""
    try:
        conn = sqlite3.connect('instance/app.db')
        conn.row_factory = sqlite3.Row  # This enables column access by name
        return conn
    except sqlite3.Error as e:
        print(f"Error connecting to database: {e}")
        sys.exit(1)

def show_tables():
    """Show all tables in the database"""
    conn = connect_db()
    cursor = conn.cursor()
    
    print("\n=== DATABASE TABLES ===")
    cursor.execute("SELECT name FROM sqlite_master WHERE type='table';")
    tables = cursor.fetchall()
    for table in tables:
        print(f"- {table[0]}")
    
    conn.close()

def show_users():
    """Show all users with their roles"""
    conn = connect_db()
    cursor = conn.cursor()
    
    print("\n=== USERS ===")
    query = """
    SELECT u.id, u.username, u.email, u.client_id, u.is_active, u.created_at,
           GROUP_CONCAT(r.name) as roles
    FROM user u
    LEFT JOIN user_roles ur ON u.id = ur.user_id
    LEFT JOIN role r ON ur.role_id = r.id
    GROUP BY u.id
    ORDER BY u.id
    """
    
    cursor.execute(query)
    users = cursor.fetchall()
    
    if not users:
        print("No users found.")
    else:
        print(f"{'ID':<3} {'Username':<15} {'Email':<25} {'Client ID':<15} {'Active':<6} {'Roles':<20} {'Created'}")
        print("-" * 100)
        for user in users:
            roles = user['roles'] if user['roles'] else 'None'
            created = user['created_at'][:19] if user['created_at'] else 'N/A'
            print(f"{user['id']:<3} {user['username']:<15} {user['email']:<25} {user['client_id']:<15} {user['is_active']:<6} {roles:<20} {created}")
    
    conn.close()

def show_roles():
    """Show all roles with their permissions"""
    conn = connect_db()
    cursor = conn.cursor()
    
    print("\n=== ROLES & PERMISSIONS ===")
    query = """
    SELECT r.id, r.name, r.description,
           GROUP_CONCAT(p.name) as permissions
    FROM role r
    LEFT JOIN role_permissions rp ON r.id = rp.role_id
    LEFT JOIN permission p ON rp.permission_id = p.id
    GROUP BY r.id
    ORDER BY r.id
    """
    
    cursor.execute(query)
    roles = cursor.fetchall()
    
    if not roles:
        print("No roles found.")
    else:
        for role in roles:
            permissions = role['permissions'] if role['permissions'] else 'None'
            print(f"\nRole: {role['name']} (ID: {role['id']})")
            print(f"Description: {role['description']}")
            print(f"Permissions: {permissions}")
    
    conn.close()

def show_permissions():
    """Show all permissions"""
    conn = connect_db()
    cursor = conn.cursor()
    
    print("\n=== PERMISSIONS ===")
    cursor.execute("SELECT * FROM permission ORDER BY resource, action")
    permissions = cursor.fetchall()
    
    if not permissions:
        print("No permissions found.")
    else:
        print(f"{'ID':<3} {'Name':<15} {'Resource':<10} {'Action':<10} {'Description'}")
        print("-" * 70)
        for perm in permissions:
            print(f"{perm['id']:<3} {perm['name']:<15} {perm['resource']:<10} {perm['action']:<10} {perm['description']}")
    
    conn.close()

def show_user_permissions(username=None):
    """Show permissions for a specific user or all users"""
    conn = connect_db()
    cursor = conn.cursor()
    
    print(f"\n=== USER PERMISSIONS {'FOR ' + username.upper() if username else ''} ===")
    
    query = """
    SELECT u.username, p.name as permission, r.name as role
    FROM user u
    JOIN user_roles ur ON u.id = ur.user_id
    JOIN role r ON ur.role_id = r.id
    JOIN role_permissions rp ON r.id = rp.role_id
    JOIN permission p ON rp.permission_id = p.id
    """
    
    params = []
    if username:
        query += " WHERE u.username = ?"
        params.append(username)
    
    query += " ORDER BY u.username, p.name"
    
    cursor.execute(query, params)
    user_perms = cursor.fetchall()
    
    if not user_perms:
        print(f"No permissions found{' for user ' + username if username else ''}.")
    else:
        current_user = None
        for perm in user_perms:
            if current_user != perm['username']:
                current_user = perm['username']
                print(f"\nUser: {current_user}")
            print(f"  - {perm['permission']} (via {perm['role']} role)")
    
    conn.close()

def main():
    """Main function"""
    print("Flask User Management Database Inspector")
    print("=" * 50)
    
    if len(sys.argv) > 1:
        command = sys.argv[1].lower()
        if command == 'users':
            show_users()
        elif command == 'roles':
            show_roles()
        elif command == 'permissions':
            show_permissions()
        elif command == 'user-perms':
            username = sys.argv[2] if len(sys.argv) > 2 else None
            show_user_permissions(username)
        elif command == 'tables':
            show_tables()
        else:
            print(f"Unknown command: {command}")
            print("Available commands: users, roles, permissions, user-perms [username], tables")
    else:
        # Show everything by default
        show_tables()
        show_users()
        show_roles()
        show_permissions()
        show_user_permissions()

if __name__ == "__main__":
    main()