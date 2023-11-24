#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NR_pcl_test  __NR_arch_specific_syscall + 14


#define SET_PCL 10
#define UNSET_PCL 11
#define LOCK_PCL 12
#define PCL_TEST_LOAD_PCODE 20
#define PCL_TEST_STORE_PCODE 21
#define PCL_TEST_EXECUTE_DATA 22
#define PCL_TEST_WRITE_PCODE_REMAP 23
#define PCL_TEST_PATCH_SYS_READ 24
#define PCL_TEST_SHUFFLE 25

struct one_test_t {
    int id;
    char* name;
};


struct one_test_t tests[] = {
    { .id = PCL_TEST_LOAD_PCODE, .name = "PCL_TEST_LOAD_PCODE", },
    { .id = PCL_TEST_STORE_PCODE, .name = "PCL_TEST_STORE_PCODE", },
    { .id = PCL_TEST_EXECUTE_DATA, .name = "PCL_TEST_EXECUTE_DATA", },
    { .id = PCL_TEST_WRITE_PCODE_REMAP, .name = "PCL_TEST_WRITE_PCODE_REMAP",},
    { .id = PCL_TEST_PATCH_SYS_READ, .name = "PCL_TEST_PATCH_SYS_READ",},
    { .id = PCL_TEST_SHUFFLE, .name = "PCL_TEST_SHUFFLE",},
};
int num_tests = sizeof(tests)/sizeof(struct one_test_t);

void test_one_with_idx(int idx) {
    int ret;
    struct one_test_t* test;
    if(idx > num_tests) {
        printf("out of bound @ %s:%d\n",__func__,__LINE__);
        exit(1);
    }
    test = &tests[idx];
    printf("[TEST - ");
    printf(test->name);
    printf("]\n");
    fflush(stdout);
    ret = syscall(NR_pcl_test, test->id);
    printf("\tReturns: %d\n",ret);
}

void test_one(int test_id) {

    int i;
    for(i = 0; i < num_tests; i+=1) {
        if (test_id == tests[i].id) {
            break;
        }
    }
    if(i == num_tests) {
        printf("failed to find test with id: %d\n",test_id);
        exit(1);
    }
    test_one_with_idx(i);

}



void enable_pcl(void) {
    int ret;
    printf("SET_PCL\n");
    fflush(stdout);
    ret = syscall(NR_pcl_test, SET_PCL);
    printf("ret: %d\n",ret);
}

void test_all(void) {
    int i = 0;
    for(i = 0; i < num_tests; i += 1) {
        test_one_with_idx(i);
    }
}

void list_tests(void) {
    int i = 0;
    printf("available tests:\n");
    for(i = 0; i < num_tests; i += 1) {
        printf("\t(%d): %s\n",tests[i].id, tests[i].name);
    }
}

void read_victim(void) {
    int fd;
    ssize_t ret;
    char buf[1024];
    fd = open("/root/victim.txt",O_RDONLY);
    ret = read(fd,buf,1024);
    printf("%d\n",ret);

}

void install_handler(void);
int main(int argc, char* argv[]) {
    int ret, opt, test_id;
    printf("pcl test driver\n");

    while((opt = getopt(argc, argv, "aelt:r")) != -1) {
        switch(opt) {
            case 'a':
                printf("running all tests\n");
                test_all();
                return 0;
            case 'e':
                printf("enabling pcl\n");
                enable_pcl();
                return 0;
            case 'l':
                list_tests();
                return 0;
            case 't':
                test_id = atoi(optarg);
                printf("%d\n",test_id);
                test_one(test_id);
                return 0;
            case 'r':
                read_victim();
                return 0;
            default:
                printf("./test [-a | -e | -t <test_id>]\n");
                return 0;
        }
    }
    printf("./test [-a | -e | -t <test_id>]\n");
    return 0;
}

