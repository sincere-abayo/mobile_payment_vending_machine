import os
import africastalking
from flask import Flask, request, jsonify

# Initialize Africa's Talking
USERNAME = "abayosincere11"
API_KEY = "atsk_14c83a11207be183dd7a74b9bdbc4b636b65e024648d86e74e8a4bb227d99ac53fb7d410"

africastalking.initialize(USERNAME, API_KEY)
sms = africastalking.SMS

app = Flask(__name__)

def send_sms(recipients, message, sender_id=None):
    """ Function to send SMS """
    try:
        response = sms.send(message, recipients, sender_id)
        return response
    except Exception as e:
        return {"error": f"SMS failed: {e}"}

@app.route("/send-sms", methods=["POST"])
def send_sms_route():
    """ API endpoint to send SMS """
    data = request.get_json()
    recipients = data.get("recipients", [])
    message = data.get("message", "Hello from Africa’s Talking!")
    sender_id = data.get("sender_id", None)

    if not recipients:
        return jsonify({"error": "Recipients required"}), 400

    response = send_sms(recipients, message, sender_id)
    return jsonify(response)

if __name__ == "__main__":
    # Test sending SMS when script runs
    test_numbers = ["+254722123123"]
    test_message = "Hey AT Ninja!"
    sender = "XXYYZZ"

    print("Sending test SMS...")
    print(send_sms(test_numbers, test_message, sender))

    # Start Flask server
    app.run(debug=True, port=int(os.environ.get("PORT", 5000)))
