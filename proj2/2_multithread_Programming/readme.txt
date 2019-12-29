1)기존 버전
gcc my_rand.c pth_ll_rwl.c -DOUTPUT -lpthread -o pth_ll_rwl
./pth_ll_rwl 6
뒤에 입력 숫자는 쓰레드 개수를 의미합니다.

2)최적화 버전
gcc my_rand.c pth_ll_rwl_opti.c -DOUTPUT -lpthread -o pth_ll_rwl_opti
./pth_ll_rwl_opti 6
뒤에 입력 숫자는 쓰레드 개수를 의미합니다.
