import os
import jwt
import time
import json
from flask import Flask, request, jsonify, make_response, send_from_directory
from urllib.request import Request, urlopen
from urllib.parse import urljoin
from Crypto.Hash import SHA256
from Crypto.PublicKey import ECC
from Crypto.Signature import DSS
from dotenv import load_dotenv
import traceback

load_dotenv()

SECRET_KEY = os.getenv("SECRET_KEY")
FLAG = os.getenv("FLAG")

app = Flask(__name__)

def get_pubkey_of_timeserver(timeserver_url):
    req = Request(urljoin(timeserver_url, "pubkey"))
    with urlopen(req) as res:
        key_text = res.read().decode("utf-8")
    return ECC.import_key(key_text)

def get_timestamp_and_signature_from_timeserver(timeserver_url):
    req = Request(urljoin(timeserver_url, "timestamp"))
    with urlopen(req) as res:
        data = json.loads(res.read().decode("utf-8"))
    return data["timestamp"], data["signature"], data["exp"]
def get_current_time_from_own_time_server():
    try:
        url = os.getenv("TIME_SERVER_URL")+"/timestamp"
        # print(f"Attempting to connect to: {url}")  # Debug print
        
        req = Request(url)
        try:
            with urlopen(req, timeout=5) as res: 
                # print("Connection successful")     
                data = json.loads(res.read().decode("utf-8"))
                # print(f"Received data: {data}")   # Debug print
                return data["timestamp"], data["signature"], data["exp"]
        except json.JSONDecodeError as e:
            # print(f"JSON decode error: {e}")
            raise
        except Exception as e:
            # print(f"Connection error: {e}")
            raise
            
    except Exception as e:
        # print(f"Time server error: {e}")
        # print(f"Full traceback: {traceback.format_exc()}")
        raise
@app.after_request
def add_cors_headers(response):
    response.headers['Access-Control-Allow-Origin'] = os.getenv("BACKEND_URL")
    response.headers['Access-Control-Allow-Methods'] = 'GET, POST, OPTIONS'
    response.headers['Access-Control-Allow-Headers'] = 'Content-Type'
    response.headers['Access-Control-Allow-Credentials'] = 'true'
    return response

@app.route("/config", methods=["GET"])
def get_config():
    return jsonify({
        "contentserver": os.getenv("BACKEND_URL"),
        "timeserver": os.getenv("TIME_SERVER_URL")+"/timestamp"
    })

@app.route("/", methods=["GET"])
@app.route("/index.html", methods=["GET"])
def serve_index():
    try:
        return send_from_directory(
            os.path.join(os.path.dirname(__file__), "..", "frontend"), "index.html"
        )
    except FileNotFoundError:
        return make_response("Not Found", 404)

@app.route("/challenge", methods=["GET", "OPTIONS"])
def challenge():
    if request.method == "OPTIONS":
        return "", 200
        
    try:
       
        try:
            current_time, signature, exp_time = get_current_time_from_own_time_server()
            # print(f"Successfully got time: {current_time}")
        except Exception as e:
            # print(f"Error getting time: {str(e)}")
            return jsonify({"error": f"Time server error: {str(e)}"}), 500
        
        # Step 2: Create payload
        try:
            payload = {
                "timestamp": current_time,
                "signature": signature,
                "timeserver": os.getenv("TIME_SERVER_URL"),
                "exp": exp_time,
            }
            print(f"Created payload: {payload}")
        except Exception as e:
            print(f"Error creating payload: {str(e)}")
            return jsonify({"error": f"Payload creation error: {str(e)}"}), 500
            

        try:
            if not SECRET_KEY:
                return jsonify({"error": "SECRET_KEY not configured"}), 500
                
            token = jwt.encode(payload, SECRET_KEY, algorithm="HS256")
            print(f"Generated token: {token}")
        except Exception as e:
            print(f"Error generating token: {str(e)}")
            return jsonify({"error": f"Token generation error: {str(e)}"}), 500
        
        try:
            response = jsonify({"token": token})
            response.set_cookie(
                "session",
                token,
                httponly=False,
                secure=True,
                samesite='None',
                path='/'
            )
            print("Successfully created response")
            return response
        except Exception as e:
            print(f"Error creating response: {str(e)}")
            return jsonify({"error": f"Response creation error: {str(e)}"}), 500

    except Exception as e:
        print(f"Unexpected error: {str(e)}")
        print(traceback.format_exc())
        return jsonify({"error": f"Server error: {str(e)}"}), 500

@app.route("/validate", methods=["POST", "OPTIONS"])
def validate():
    if request.method == "OPTIONS":
        return "", 200
        
    try:
        data = request.get_json()
        token = data.get("token") or request.cookies.get("session")
        
        if not token:
            return jsonify({"error": "Invalid request - No token provided"}), 400

        try:
            payload = jwt.decode(token, SECRET_KEY, algorithms=["HS256"], options={"verify_signature": False})
            
            timeserver_url = payload.get("timeserver")
            if not timeserver_url:
                return jsonify({"error": "Invalid token payload - No timeserver URL"}), 400
            
            try:
                timestamp, signature, exp_time = get_timestamp_and_signature_from_timeserver(timeserver_url)

            except Exception as e:

                return jsonify({"error": f"Error communicating with timeserver: {str(e)}"}), 500

            try:
                pubkey = get_pubkey_of_timeserver(timeserver_url)
            except Exception as e:
                return jsonify({"error": f"Error getting public key: {str(e)}"}), 500

            try:
                h = SHA256.new(timestamp.encode("utf-8"))
                verifier = DSS.new(pubkey, "fips-186-3")
                verifier.verify(h, bytes.fromhex(signature))
            except ValueError as e:
                return jsonify({"error": "The signature is not authentic"}), 401
            except Exception as e:
                return jsonify({"error": f"Error verifying signature: {str(e)}"}), 500

            current_time = int(time.time())
            if int(exp_time) <= current_time:
                return jsonify({"error": "Token has expired"}), 401

            return jsonify({"message": "Token is valid", "flag": FLAG})

        except jwt.ExpiredSignatureError:
            return jsonify({"error": "Token has expired"}), 401
        except jwt.InvalidTokenError as e:
            return jsonify({"error": "Invalid token"}), 401

    except Exception as e:
        return jsonify({"error": f"Server error: {str(e)}"}), 500

@app.route("/<path:filename>", methods=["GET"])
def serve_static(filename):
    try:
        return send_from_directory(
            os.path.join(os.path.dirname(__file__), "..", "frontend"), filename
        )
    except FileNotFoundError:
        return make_response("Not Found", 404)

@app.errorhandler(Exception)
def handle_global_error(e):
    error_message = f"Global Error: {traceback.format_exc()}"
    return jsonify({"error": "Internal Server Error"}), 500

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5002, debug=True)
