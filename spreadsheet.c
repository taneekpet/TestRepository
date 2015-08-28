#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

double expression();

double *spreadsheet[26];
int length[26],flag,position;
char buf[1024];

void sig_handler(int signum) {
    switch(signum) {

        case SIGQUIT:
        for(int i = 0 ; i < 26 ; i++) {
            if(length[i] != 0) free(spreadsheet[i]);
        }
        printf(" Cleanly terminated.\n");
        exit(0);

        case SIGHUP: printf(" SIGHUP recieved, ignored.\n");break;
        case SIGINT: printf(" SIGINT recieved, ignored.\n");break;
        default: printf(" Unexpected signal, terminated.\n");
        exit(1);
    }
}

void remove_lead_spaces() {
    while(buf[position] == ' ' || buf[position] == '\t') position++;
}

int is_end(char x) {
    return (x == '\n') || (x == ' ') || (x == '\t');
}

double number() {
//printf("num %d%s",position,buf);
    if(flag != 0) return -1;
    remove_lead_spaces();
    double n = 0,man=0;
    int dec = 0;
    while(buf[position] != '.') {
        if(buf[position] >= '0' && buf[position] <= '9') {
            n *= 10;
            n += buf[position] - '0';
            position++;
        }
        else if(is_end(buf[position])) return n;
        else {
            flag = 1;
            return -1;
        }
    }
    position++; //skip period
    while(!is_end(buf[position])) {
        if(buf[position] >= '0' && buf[position] <= '9') {
            man += (double)(buf[position]-'0')/pow(10,++dec);
            position++;
        }
        else {
            flag = 1;
            return -1;
        }
    }
    return n+man;
}

double cell(int assignment) {
//printf("asgmnt %d%s",position,buf);
    if(flag != 0) return -1;
    if(assignment) position++;          //skip equal sign
    remove_lead_spaces();
    int row,column = 0;
    row = buf[position] - 'A';
    while(!is_end(buf[position])) {
        column *= 10;
        column += buf[position] - '0';
        position++;
    }
    if(length[row] == 0) {
        spreadsheet[row] = malloc(sizeof(double)*(column+1));
        if(spreadsheet[row] == NULL) {
            flag = 3;
            return -1;
        }
        length[row] = column+1;
        *(spreadsheet[row]+column) = 0;
    }
    else if(length[row] < column) {
        double *tmp;
        tmp = realloc(spreadsheet[row],sizeof(double)*(column+1));
        if(tmp == NULL) {
            flag = 3;
            return -1;
        }
        else {
            spreadsheet[row] = tmp;
            length[row] = column+1;
            *(spreadsheet[row]+column) = 0;
        }
    }
    if(assignment) spreadsheet[row][column] = expression();
    return spreadsheet[row][column];
}

double expression() {
//printf("exp %d%s",position,buf);
    if(flag != 0) return -1;
    remove_lead_spaces();
    double x,y;
    while(!is_end(buf[position])) {    

        if(buf[position] >= 'A' && buf[position] <= 'Z')                            return cell(0);
        if(buf[position] == '.' || (buf[position] >= '0' && buf[position] <= '9'))  return number();

        else if(buf[position] != '+' && buf[position] != '-' && buf[position] != '*' && buf[position] != '/') {
            flag = 1;
            return -1;
        }

        else {
                char sign = buf[position];
                position++;
                x = expression();
                y = expression();
                switch(sign) {
                    case '+': return x+y;
                    case '-': return x-y;
                    case '*': return x*y;
                    case '/': return x/y;
            }
        }
    }
    flag = 2;
    return -1;
}

double command() {
//printf("cmd %d%s",position,buf);
    while(buf[position] != '\n') {
        remove_lead_spaces();
        if(buf[position] >= 'A' && buf[position] <= 'Z')                                                        return expression();
        else if(buf[position] == '.' || (buf[position] >= '0' && buf[position] <= '9'))                         return expression();
        else if(buf[position] == '+' || buf[position] == '-' || buf[position] == '*' || buf[position] == '/')   return expression();
        else if(buf[position] == '=')                                                                           return cell(1);
        else {
            flag = 1;
            return -1;
        }
    }
    flag = 2;
    return -1;
}

int main() {
    if(signal(SIGQUIT,sig_handler) == SIG_ERR) {
        printf("Can not catch SIG_QUIT.\n");
        return 1;
    }
    if(signal(SIGHUP,sig_handler) == SIG_ERR) {
        printf("Can not catch SIG_HUP.\n");
        return 1;
    }
    if(signal(SIGINT,sig_handler) == SIG_ERR) {
        printf("Can not catch SIG_INT.\n");
        return 1;
    }

    double result;
    for(int i = 0 ; i < 26 ; i++) length[i] = 0;

    //printf("\n====================================================================\n");
    //printf("1) Each token seperate by space.\n");
    //printf("2) Support only prefix operation (including assignment).\n");
    //printf("3) Reference to cell by capital character and number, e.g. C12.\n");
    //printf("4) Empty cell contains zero.\n");
    //printf("====================================================================\n\n");

    while(1) {
        fgets(buf,255,stdin);
        flag = 0; position = 0;
        result = command();
        switch(flag) {
            case 0: printf("Result: %lf\n",result); break;
            case 1: printf("Error: unrecognized command.\n"); break;
            case 2: printf("Error: too few argument somewhere.\n"); break;
            case 3: printf("Memory error.\n"); break;
            default: printf("Fatal error: recieved unrecognized flag #%d, terminated.\n",flag); exit(1);
        }
    }
    return 1;
}
