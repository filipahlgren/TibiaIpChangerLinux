#include <iostream>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <map>

#include "values.h"

std::string ip = "127.0.0.1";
std::string version = "860";
std::string processName = "Tibia";
unsigned int port = 7171;

void help();
int setArgs(int argc, char** argv);
int writeToMem(int fd, long int pid, int value, off_t addr);
int writeToMem(int fd, long int pid, const char* value, off_t addr);
int fdFromName(std::string name, long int &pid);
int writeToProcess(int fd, long int pid);

int main(int argc, char** argv){

	std::cout << "Welcome to bullz tibia ip changer" << std::endl << std::endl;

	setArgs(argc, argv);
	std::cout << "IP: " << ip << std::endl;
	std::cout << "Port: " << port << std::endl;
	std::cout << "Version: " << version << std::endl;
	std::cout << "Process name: " << processName << std::endl << std::endl;

	long int pid = 0;
	int fd = fdFromName("Tibia", pid);

	if (pid == 0){
		std::cout << "Process not found" << std::endl;
		close(fd);
		return 0;
	}

	int writeStatus = writeToProcess(fd, pid);
	if(writeStatus == -1) {
		std::cout << "Version not found" << std::endl;
		close(fd);
		return 0;
	}

	std::cout << "Ip changed" << std::endl;

	close(fd);

	return 0;
}

void help(){
	std::cout << "Commands:" << std::endl;
	std::cout << "\t-s, --server: Server IP-address" << std::endl;
	std::cout << "\t-p, --port: Server port number" << std::endl;
	std::cout << "\t-v, --version: Version of tibia client" << std::endl;
	std::cout << "\t-h, --help: This help menu" << std::endl << std::endl;
	std::cout << "Example:" << std::endl;
	std::cout << "\t./example -s 127.0.0.1 -p 7171 -v 860 -n Tibia" << std::endl;
	std::cout << std::endl;
}

int setArgs(int argc, char** argv){
	for(int i = 1; i < argc; i+=2){

		if (strcmp("-h", argv[i]) == 0 ||
			strcmp("--help", argv[i]) == 0){
				help();
				exit(1);
		}

		if (i + 1 >= argc) return -1;

		if (strcmp("-s", argv[i]) == 0 ||
			strcmp("--server", argv[i]) == 0){
				ip = argv[i+1];
				if(ip.length() > 64) {
					std::cout << "Ip too long" << std::endl;
					exit(1);
				}
		}

		if (strcmp("-p", argv[i]) == 0 ||
			strcmp("--port", argv[i]) == 0){
				port = std::stoi(argv[i+1]);
		}

		if (strcmp("-v", argv[i]) == 0 ||
			strcmp("--version", argv[i]) == 0){
				version = argv[i+1];
		}

	}
	return 1;
}

int writeToProcess(int fd, long int pid){
	int *list = versionMap[version.c_str()];
	if (list == NULL) return -1;

	//Write ip and port
	off_t baseAddress = (off_t)list[0];
	for (int i = 0; i < list[3]; i++){
		writeToMem(fd, pid, ip.c_str(), baseAddress);
		writeToMem(fd, pid, port, baseAddress + list[2]);
		baseAddress += (off_t)list[1];
	}

	//write RSA
	writeToMem(fd, pid, rsaKey, (off_t)list[4]);
	return 1;
}

int writeToMem(int fd, long int pid, int value, off_t addr){
	ptrace(PTRACE_ATTACH, pid, 0, 0);
	waitpid(pid, NULL, 0);

	int ret = pwrite(fd, &value, sizeof(value), addr);

	ptrace(PTRACE_DETACH, pid, 0, 0);

	return ret;
}

int writeToMem(int fd, long int pid, const char* value, off_t addr){
	ptrace(PTRACE_ATTACH, pid, 0, 0);
	waitpid(pid, NULL, 0);

	char empty[64] = "";
	pwrite(fd, empty, sizeof(empty), addr);

	int ret = pwrite(fd, value, (strlen(value)*sizeof(*value)), addr);

	ptrace(PTRACE_DETACH, pid, 0, 0);

	return ret;
}

int fdFromName(std::string name, long int &pid){
	int LEN = 200;
	char line[LEN];
	std::string command = "pidof -s Tibia";
	FILE *cmd = popen(command.c_str(), "r");

	fgets(line, LEN, cmd);
	pid = strtoul(line, NULL, 10);

	pclose(cmd);

	if (pid == 0) return -1;

	char file[64];
	sprintf(file, "/proc/%ld/mem", (long)pid);

	return open(file, O_RDWR);
}
