from flask import Flask, jsonify, Response
from Crypto.Hash import SHA256
from Crypto.PublicKey import ECC
from Crypto.Signature import DSS
import time
import os
from dotenv import load_dotenv
from flask import request, make_response
load_dotenv()
app = Flask(__name__)

key = ECC.generate(curve="p256")
pubkey = key.public_key().export_key(format="PEM")


@app.route("/pubkey", methods=["GET"])
def get_pubkey():
  
    response = Response(pubkey, mimetype="text/plain")
    response.headers["Access-Control-Allow-Origin"] = "*"
    return response


@app.route("/timestamp", methods=["GET","OPTIONS"])
def get_timestamp():
    if request.method == "OPTIONS":
       response = make_response()
       response.headers.add("Access-Control-Allow-Origin", os.getenv("BACKEND_URL"))
       response.headers.add("Access-Control-Allow-Methods", "GET")
       response.headers.add("Access-Control-Allow-Headers", "Content-Type")
       response.headers.add("Access-Control-Allow-Credentials", "true")
       return response      
    current_time = str(int(time.time()))
    exp_time = str(int(time.time()) - 5)  

    h = SHA256.new(current_time.encode("utf-8"))
    signer = DSS.new(key, "fips-186-3")
    signature = signer.sign(h)

    response_data = {
        "timestamp": current_time,
        "signature": signature.hex(),
        "exp": exp_time,
    }

    response = jsonify(response_data)
    response.headers["Access-Control-Allow-Origin"] = os.getenv("BACKEND_URL")
    response.headers["Access-Control-Allow-Credentials"] = "true"

    return response


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5001)
