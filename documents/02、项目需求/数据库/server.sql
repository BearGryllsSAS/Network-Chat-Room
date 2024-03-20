-- ----------------------------
--  Create database and change database
-- ----------------------------
DROP DATABASE IF EXISTS `server`;
CREATE DATABASE `server` CHARACTER SET 'utf8mb4' COLLATE 'utf8mb4_0900_ai_ci';
USE `server`;

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for relation_base_portfolio
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE user (
    id INT AUTO_INCREMENT PRIMARY KEY,
    usr_id INT NOT NULL,
    usr_name VARCHAR(255) NOT NULL,
    usr_password VARCHAR(255) NOT NULL
);

-- --------------------------
-- Insert data into user 
-- --------------------------
INSERT INTO user(usr_id, usr_name, usr_password) VALUES('00000', 'user0', '123');
INSERT INTO user(usr_id, usr_name, usr_password) VALUES('11111', 'user1', '123');
INSERT INTO user(usr_id, usr_name, usr_password) VALUES('22222', 'user2', '123');
INSERT INTO user(usr_id, usr_name, usr_password) VALUES('33333', 'user3', '123');
INSERT INTO user(usr_id, usr_name, usr_password) VALUES('44444', 'user4', '123');
INSERT INTO user(usr_id, usr_name, usr_password) VALUES('55555', 'user5', '123');
INSERT INTO user(usr_id, usr_name, usr_password) VALUES('66666', 'user6', '123');
INSERT INTO user(usr_id, usr_name, usr_password) VALUES('77777', 'user7', '123');
INSERT INTO user(usr_id, usr_name, usr_password) VALUES('88888', 'user8', '123');
INSERT INTO user(usr_id, usr_name, usr_password) VALUES('99999', 'user9', '123');