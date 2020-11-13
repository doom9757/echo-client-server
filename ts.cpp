#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <thread>
#include <set>
#include <string>

using namespace std;
int num=0;
set<int> str;
set<int>::iterator iter;
//char broadmessage[65536];
void usage() {
	cout << "syntax: ts [-an][-e] <port>\n";
	cout << "  -an: auto newline\n";
	cout << "  -e : echo\n";
	cout << "  -d : broadcast\n";
	cout << "sample: ts 1234\n";
}

struct Param {
	bool autoNewline{false};
	bool echo{false};
	bool broadcast{false};
	uint16_t port{0};

	bool parse(int argc, char* argv[]) {
		port = stoi(argv[1]);
		for (int i = 1; i < argc; i++) {
			//printf("%d\n",i);
			if (strcmp(argv[i], "-an") == 0) {
				autoNewline = true;
				continue;
			}
			if (strcmp(argv[i], "-e") == 0) {
				echo = true;
				continue;
			}
			if ( strcmp(argv[i],"-b") == 0){
				broadcast = true;
				continue;
			}
			//port = stoi(argv[i++]);
		}
		return port != 0;
	}
} param;

void recvThread(int sd) {
	cout << "connected\n";
	//cout << param.echo << param.broadcast;
	static const int BUFSIZE = 65536;
	char buf[BUFSIZE];
	memset(buf,0,65536);
	while (true) {
		ssize_t res;
		//cout<<broadmessage<<endl<<buf<<endl;
		/*if(param.broadcast&&(memcmp(buf,broadmessage,65536))){
			res = send(sd, broadmessage, strlen(broadmessage), 0);
			if (res == 0 || res == -1) {
				//cout<<res;
				cerr << "send return " << res << endl;
				perror("send");
				break;
			}
			memcpy(buf,broadmessage,65536);
			continue;
		}*/
		res = recv(sd, buf, BUFSIZE - 1, 0);
		if (res == 0 || res == -1) {
			cerr << "recv return " << res << endl;
			perror("recv");
			break;
		}
		for(int i=res;i<65536;i++){
			buf[i] = '\0';
		}
		if (param.autoNewline)
			cout << "message : " << buf << endl;
		else {
			cout << "message : " << buf;
			cout.flush();
		}
		if (param.echo&&(!param.broadcast)) {
			res = send(sd, buf, res, 0);
			//cout<<res;
			if (res == 0 || res == -1) {
				//cout<<res;
				cerr << "send return " << res << endl;
				perror("send");
				//break;
			}
		}
		else if(param.echo&&param.broadcast){
			for(iter = str.begin();iter!=str.end();iter++){
				res = send(*iter, buf, res, 0);
				if (res == 0 || res == -1) {
				//cout<<res;
					cerr << "send return " << res << endl;
					perror("send");
					//break;
				}
				cout << "broadcast success!\n";
			}
			//res = send(sd, buf, res, 0);
			//cout<<res;
			/*if (res == 0 || res == -1) {
				//cout<<res;
				cerr << "send return " << res << endl;
				perror("send");
				//break;
			}*/
			//memcpy(broadmessage,buf,65536);
			//cout<<broadmessage<<endl<<buf<<endl;
		}
		//memcpy(broadmessage,buf,65536);
	}
	cout << "disconnected\n";
	str.erase(str.find(sd));
    close(sd);
}

int main(int argc, char* argv[]) {
	//memset(broadmessage,0,65536);
	if (!param.parse(argc, argv)) {
		usage();
		return -1;
	}
	//param.broadcast = true;
	//cout << param.echo << param.broadcast;
	//cout << param.port;
	if((!param.echo)&&(param.broadcast)){
		cout << "Without echo option, cannot broadcast\n";
	}
	cout << "newline : " << (param.autoNewline?"True":"False") << endl;
	cout << "echo mode : " << (param.echo?"True":"False") << endl;
	cout << "broadcast mode : " << (param.broadcast?"True":"False") << endl;
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket");
		return -1;
	}

	int optval = 1;
	int res = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res == -1) {
		perror("setsockopt");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(param.port);

	ssize_t res2 = ::bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (res2 == -1) {
		perror("bind");
		return -1;
	}

	res = listen(sd, 5);
	if (res == -1) {
		perror("listen");
		return -1;
	}
	while (true) {
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		int cli_sd = accept(sd, (struct sockaddr *)&cli_addr, &len);
		if (cli_sd == -1) {
			perror("accept");
			break;
		}
		str.insert(cli_sd);
		thread* t = new thread(recvThread, cli_sd);
		t->detach();
	}
	close(sd);
}
