from flask import Flask, request, jsonify
from flask_restful import Api, Resource
import json
from auth import token_required, get_token_route

app = Flask(__name__)
api = Api(app)

# Sample in-memory database
items_db = {}
item_id_counter = 0

# Item Resource
class ItemResource(Resource):
    @token_required
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
    def delete(self, item_id):
        if item_id not in items_db:
            return {"error": "Item not found"}, 404
            
        del items_db[item_id]
        return "", 204

# Register resources
api.add_resource(ItemResource, '/items', '/items/<int:item_id>')

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