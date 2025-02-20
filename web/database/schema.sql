CREATE DATABASE vending_machine;
USE vending_machine;

CREATE TABLE dispensing_records (
    id INT AUTO_INCREMENT PRIMARY KEY,
    transaction_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    phone_number VARCHAR(15),
    liters_dispensed DECIMAL(10,2),
    amount_paid DECIMAL(10,2),
);

