@echo off
start DemoServer.exe   127.0.0.1    8081
start DemoServer.exe   127.0.0.1    8082
start DemoServer.exe   127.0.0.1    8083
start DemoServer.exe   127.0.0.1    8084

start LoginServer.exe   127.0.0.1    8080
exit