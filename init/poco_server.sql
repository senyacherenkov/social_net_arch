-- DROP DATABASE IF EXISTS social_net;
-- CREATE DATABASE social_net;
-- USE social_net;

-- GRANT USAGE ON *.* TO 'poco_server'@'localhost';
-- DROP USER 'poco_server'@'localhost';
-- FLUSH PRIVILEGES;

-- CREATE USER 'poco_server'@'localhost' IDENTIFIED BY 'otus';
-- GRANT ALL PRIVILEGES ON * . * TO 'poco_server'@'localhost';
-- FLUSH PRIVILEGES;

DROP TABLE IF EXISTS creds;
DROP TABLE IF EXISTS profile;
DROP TABLE IF EXISTS friends;

CREATE TABLE creds 
    (id             INT unsigned NOT NULL AUTO_INCREMENT,
    login           VARCHAR(150) NOT NULL,
    pwd             VARCHAR(150) NOT NULL,
    PRIMARY KEY     (id));

CREATE TABLE profile 
    (id               INT unsigned NOT NULL AUTO_INCREMENT,
        crid            INT unsigned NOT NULL,
        fname           VARCHAR(150) NOT NULL,
        sname           VARCHAR(150) NOT NULL,
        age             INT unsigned NOT NULL,
        gender          VARCHAR(150) NOT NULL,
        hob             VARCHAR(150) NOT NULL,
        city            VARCHAR(150) NOT NULL,
        PRIMARY KEY     (id));

CREATE TABLE friends 
    (id             INT unsigned NOT NULL AUTO_INCREMENT,
    userid          INT unsigned NOT NULL,
    frid            INT unsigned NOT NULL,
    PRIMARY KEY     (id));
