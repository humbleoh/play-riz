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

## Available Endpoints

- `GET /`: Welcome message
- `POST /items`: Create a new item
- `GET /items`: List all items
- `GET /items/<item_id>`: Get a specific item
- `PUT /items/<item_id>`: Update an item
- `DELETE /items/<item_id>`: Delete an item

## Example Usage

### Create an item

```bash
curl -X POST "https://localhost:8000/items" -H "Content-Type: application/json" -d '{"name": "Laptop", "description": "High-performance laptop", "price": 999.99, "tax": 10.0}' --insecure
```

### Get all items

```bash
curl -X GET "https://localhost:8000/items" --insecure
```

### Get a specific item

```bash
curl -X GET "https://localhost:8000/items/1" --insecure
```

### Update an item

```bash
curl -X PUT "https://localhost:8000/items/1" -H "Content-Type: application/json" -d '{"name": "Updated Laptop", "description": "High-performance laptop with updates", "price": 1099.99, "tax": 10.0}' --insecure
```

### Delete an item

```bash
curl -X DELETE "https://localhost:8000/items/1" --insecure
```