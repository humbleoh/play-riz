# UV RESTful API

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

This API uses OAuth 2.0 Client Credentials flow for machine-to-machine (M2M) authentication. All item endpoints require a valid access token.

### Getting an Access Token

```bash
curl -X POST "https://localhost:8000/oauth/token" \
  -H "Content-Type: application/json" \
  -d '{"client_id": "your_client_id", "client_secret": "your_client_secret", "grant_type": "client_credentials"}' \
  --insecure
```

**Note**: Tokens expire after 5 minutes for security purposes.

## Available Endpoints

- `GET /`: Welcome message (no authentication required)
- `POST /oauth/token`: Get access token for authentication
- `POST /items`: Create a new item (requires authentication)
- `GET /items`: List all items (requires authentication)
- `GET /items/<item_id>`: Get a specific item (requires authentication)
- `PUT /items/<item_id>`: Update an item (requires authentication)
- `DELETE /items/<item_id>`: Delete an item (requires authentication)

## Example Usage

### 1. Get an Access Token

```bash
curl -X POST "https://localhost:8000/oauth/token" \
  -H "Content-Type: application/json" \
  -d '{"client_id": "your_client_id", "client_secret": "your_client_secret", "grant_type": "client_credentials"}' \
  --insecure
```

Response:
```json
{
  "access_token": "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9...",
  "token_type": "Bearer",
  "expires_in": 300
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