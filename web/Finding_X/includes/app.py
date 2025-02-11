from flask import Flask, request, render_template, jsonify, abort, session
from lxml import etree
from dotenv import load_dotenv
load_dotenv()
import os
import logging


app = Flask(__name__, template_folder='templates', static_folder='static')
app.secret_key = os.getenv('FLASK_SECRET_KEY')  


xml_data = """
<company>
    <department id="1" name="Confidential">
        <employee>
            <name>Confidential</name>
            <id>EMP007</id>
            <details>
                <position>Confidential</position>
                <selfDestructCode>p_ctf{i_h4t3_br97f0r63_</selfDestructCode>
            </details>
        </employee>
    </department>
</company>
"""

@app.route("/")
def home():
    if 'username' not in session:
        session['username'] = 'guest'
    return render_template("index.html")

@app.route("/about")
def about():
    return render_template("about.html")

@app.route("/features")
def features():
    return render_template("features.html")

@app.route("/admin")
def admin():
    xff = request.headers.get("X-Forwarded-For", "").split(",")[0].strip()  
    if xff != "127.0.0.1":
         abort(403)

    if session.get('username') == 'admin':
        return jsonify({
            "message": "Welcome Admin!", 
            "flag_part_2": "b4d_4nd_i_c4n_n0t_l13}"
        })
    return render_template("admin.html")

@app.route("/api/search", methods=["POST"])
def search():
    try:
        try:
            data = request.get_json(silent=True)
            print(data)
            if not data or not isinstance(data, dict):
                return jsonify({"message": "Invalid JSON format."}), 400
        except Exception:
            return jsonify({"message": "Invalid JSON format."}), 400

        if "search" not in data:
            return jsonify({"message": "Missing 'search' parameter."}), 400

        query = data.get("search", "")
        if not isinstance(query, str):
            return jsonify({"message": "Search query must be a string."}), 400

        query = query.strip()
        if not query:
            return jsonify({"message": "Search query cannot be empty."}), 400

        if any(char in query for char in ['\0', '\r', '\n']):
            return jsonify({"message": "Invalid characters in search query."}), 400
        
        blocked_terms = [
            'concat(', 'string(', 'name(',
            '//selfDestructCode', './/selfDestructCode',
            'count(', 'position(', 'last(',
            'normalize-space', 'translate'
        ]
        
        if any(term.lower() in query.lower() for term in blocked_terms):
            return jsonify({"message": "Invalid search query."}), 400

        if len(query) > 500:
            return jsonify({"message": "Search query too long."}), 400

        try:
           
            tree = etree.fromstring(xml_data, parser=etree.XMLParser(resolve_entities=False))
            
            injection_patterns = ["' or 1=1", "' or 'a'='a", "' or 1=1 or 'a'='a"]
            if any(pattern in query for pattern in injection_patterns):
                return jsonify({"message": "Employee exists."}), 200

            try:
                xpath_query = f"//selfDestructCode[starts-with(text(), '{query}')]"
                result = tree.xpath(xpath_query)
                
                return jsonify({
                    "message": "Employee exists." if result else "Employee doesn't exist."
                }), 200

            except etree.XPathEvalError:
                return jsonify({"message": "Employee doesn't exist."}), 200
            except etree.XPathSyntaxError:
                return jsonify({"message": "Employee doesn't exist."}), 200

        except etree.XMLSyntaxError:
            return jsonify({"message": "An error occurred processing the request."}), 500
        except Exception:
            return jsonify({"message": "An error occurred processing the request."}), 500

    except Exception as e:
        app.logger.error(f"Unexpected error in search endpoint: {str(e)}")
        return jsonify({"message": "An unexpected error occurred."}), 500

@app.errorhandler(400)
def bad_request(error):
    return jsonify({"message": "Bad request."}), 400

@app.errorhandler(404)
def not_found(error):
    return jsonify({"message": "Resource not found."}), 404

@app.errorhandler(405)
def method_not_allowed(error):
    return jsonify({"message": "Method not allowed."}), 405


@app.errorhandler(403)
def forbidden_error(e):
    app.logger.error(f"403 Forbidden: {e}")
    return render_template("error.html", message="Access Denied"), 403


@app.errorhandler(500)
def internal_server_error(error):
    return jsonify({"message": "An unexpected server error occurred."}), 500

@app.errorhandler(Exception)
def handle_exception(e):
    app.logger.error(f"Unhandled exception: {str(e)}")
    return jsonify({"message": "An unexpected server error occurred."}), 500


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5001, debug=False)
