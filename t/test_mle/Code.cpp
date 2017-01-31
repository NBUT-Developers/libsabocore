#include <bits/stdc++.h>
using namespace std;

char val[4], hua[4];
long long AELX[80080800];
int temp[10];
int main() {
    int t;
    for (t = 0; t < 80080800; t++) {
        AELX[t] = -1;
    }
    scanf("%d", &t);
    char c = 0;
    while (t--) {
        bool flag = 1;
        for (int i = 0; i < 5; ++i) {
            scanf("%s%s", val, hua);
            if (!flag) {
                continue;
            }
            int len = strlen(val);
            if (len == 2) {
                temp[i] = 10;
            }
            else {
                if (val[0] <= '9' && val[0] >= '0') {
                    temp[i] = val[0] - '0';
                }
                else {
                    if (val[0] == 'J') {
                        temp[i] = 11;
                    }
                    else if (val[0] == 'Q') {
                        temp[i] = 12;
                    }
                    else if (val[0] == 'K') {
                        temp[i] = 13;
                    }
                    else {
                        temp[i] = 14;
                    }
                }
            }
            if (i == 0) {
                c = hua[0];
            }
            else {
                if (c != hua[0]) {
                    flag = 0;
                }
            }
        }
        if (!flag) {
            printf("No\n");
        }
        else {
            sort(temp, temp + 5);
            if (temp[0] == temp[1] - 1 && temp[1] == temp[2] - 1 
                && temp[2] == temp[3] - 1 && temp[3] == temp[4] - 1) {
                printf("Yes\n");
            }
            else {
                if (temp[0] == 2 && temp[1] == 3 && temp[2] == 4 && temp[3] == 5 && temp[4] == 14) {
                    printf("Yes\n");
                }
                else {
                    printf("No\n");
                }
            }
        }
    }
    return 0;
}
