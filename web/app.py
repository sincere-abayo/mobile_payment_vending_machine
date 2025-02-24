
import os
import requests
import africastalking
from flask import Flask, render_template, jsonify, request
from flask_mysqldb import MySQL
from datetime import datetime, timedelta
import calendar

# Add Africa's Talking credentials
AT_USERNAME = "Iot_project"
AT_API_KEY = "atsk_6ccbe2174a56e50490d59c73c1f7177fc02e47c2cdecb5343b67e6680bc321677b10c4bd"

# Initialize Africa's Talking
africastalking.initialize(AT_USERNAME, AT_API_KEY)
sms = africastalking.SMS

def send_transaction_sms(phone_number, amount, liters):
    try:
        message = f"Payment of {amount} RWF received for {liters}L of water. Thank you for using our service!"
        print(f"Attempting to send SMS to {phone_number}")
        print(f"Using credentials - Username: {AT_USERNAME}, API Key: {AT_API_KEY}")
        response = sms.send(message, [phone_number])
        print(f"SMS Response: {response}")
        return True
    except Exception as e:
        print(f"Detailed SMS Error: {str(e)}")
        print(f"Error type: {type(e)}")
        return False

    
app = Flask(__name__)

# MySQL configurations
app.config['MYSQL_HOST'] = 'localhost'
app.config['MYSQL_USER'] = 'root'
app.config['MYSQL_PASSWORD'] = 'root'
app.config['MYSQL_DB'] = 'vending_machine'

mysql = MySQL(app)

@app.route('/')
def dashboard():
    return render_template('dashboard.html')

@app.route('/api/daily-stats')
def daily_stats():
    cur = mysql.connection.cursor()
    cur.execute("""
        SELECT DATE(transaction_date) as date, 
               SUM(liters_dispensed) as total_liters,
               SUM(amount_paid) as total_amount
        FROM dispensing_records
        WHERE transaction_date >= DATE_SUB(CURDATE(), INTERVAL 7 DAY)
        GROUP BY DATE(transaction_date)
        ORDER BY date
    """)
    data = cur.fetchall()
    cur.close()
    return jsonify(data)

@app.route('/api/monthly-stats/<int:year>')
def monthly_stats(year):
    cur = mysql.connection.cursor()
    cur.execute("""
        SELECT MONTH(transaction_date) as month,
               SUM(liters_dispensed) as total_liters,
               SUM(amount_paid) as total_amount
        FROM dispensing_records
        WHERE YEAR(transaction_date) = %s
        GROUP BY MONTH(transaction_date)
        ORDER BY month
    """, (year,))
    data = cur.fetchall()
    cur.close()
    return jsonify(data)

@app.route('/api/hourly-stats')
def hourly_stats():
    cur = mysql.connection.cursor()
    cur.execute("""
        SELECT HOUR(transaction_date) as hour,
               SUM(liters_dispensed) as total_liters,
               SUM(amount_paid) as total_amount
        FROM dispensing_records
        WHERE DATE(transaction_date) = CURDATE()
        GROUP BY HOUR(transaction_date)
        ORDER BY hour
    """)
    data = cur.fetchall()
    cur.close()
    return jsonify(data)

@app.route('/api/today-transactions')
def today_transactions():
    cur = mysql.connection.cursor()
    cur.execute("""
        SELECT transaction_date,
               phone_number,
               liters_dispensed,
               amount_paid
        FROM dispensing_records
        WHERE DATE(transaction_date) = CURDATE()
        ORDER BY transaction_date DESC
    """)
    data = cur.fetchall()
    cur.close()
    return jsonify(data)

@app.route('/api/summary-stats')
def summary_stats():
    cur = mysql.connection.cursor()
    cur.execute("""
        SELECT 
            SUM(liters_dispensed) as total_liters,
            SUM(amount_paid) as total_amount,
            COUNT(DISTINCT phone_number) as unique_customers,
            AVG(liters_dispensed) as avg_consumption
        FROM dispensing_records
        WHERE DATE(transaction_date) = CURDATE()
    """)
    data = cur.fetchone()
    cur.close()
    return jsonify({
        'total_liters': float(data[0]) if data[0] else 0,
        'total_amount': float(data[1]) if data[1] else 0,
        'unique_customers': int(data[2]) if data[2] else 0,
        'avg_consumption': float(data[3]) if data[3] else 0
    })

# record dispensing records
@app.route('/api/record-transaction', methods=['POST'])
def record_transaction():
    cur = mysql.connection.cursor()
    data = request.json
    
    cur.execute("""
        INSERT INTO dispensing_records 
        (phone_number, liters_dispensed, amount_paid)
        VALUES (%s, %s, %s)
    """, (
        data['phone_number'],
        data['liters_dispensed'],
        data['amount_paid']
    ))
    
    mysql.connection.commit()
    cur.close()

    # Send SMS notification
    # sms_sent = send_transaction_sms(
    #     data['phone_number'],
    #     data['amount_paid'],
    #     data['liters_dispensed']
    # )
    sms_sent = True

    return jsonify({
        "status": "success",
        "sms_sent": sms_sent
    })


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
