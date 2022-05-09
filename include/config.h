#ifndef __CONFIG_H__
#define __CONFIG_H__

const unsigned int MAGIC = 0x05;

// 内网地址
const std::string SERVER_I_IP = "127.0.0.1";

// 外网地址
// const std::string SERVER_O_IP = "119.23.142.60";
const std::string SERVER_O_IP = "127.0.0.1";

const int PORT = 9999;
const int FILEPORT = 9998;

const std::string SQL_PATH = "/home/miaohn/codes/chatroom/sql/create_table.sql";

#endif  // __CONFIG_H__