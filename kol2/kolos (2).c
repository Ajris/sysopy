 struct timeval curr_time(){
    struct timeval t;
    gettimeofday(&t, NULL);
    return t;
}

long int time_diff(struct timeval t1, struct timeval t2){
    return (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;
}

void print_time(struct timeval t){
    printf("%ld.%ld", t.tv_sec, t.tv_usec);
}

char *home = getenv("HOME");


----------KOLEJKI----------
---SysV---
key_t ftok(char *pathname, char proj);
int msgget(key_t key, int msgflg);
int msgctl(int msgqid, int cmd, struct msqid_ds *buf);

int msgsnd(int msqid, struct msgbuf *msgp, int msgsz,int msgflg);
int msgrcv(int msgqid, struct msgbuf *msgp, int msgsz, long type, int msgflg);

struct msgbuf {
    long mtype;
    char mtext[1];
};

-

key_t key = ftok(home_dir, PROJECT_ID);
if(key == (key_t) -1) //error
tworzenie -> int queue_id = msgget(key, IPC_CREAT | IPC_EXCL | 0777);
odbieranie utworzonej -> int server_queue_id = msgget(key, 0);
usuwanie -> msgctl(queue_id, IPC_RMID, NULL);


Message msg;
wysylanie wiad -> msgsnd(queue_id,&msg,sizeof(Message)-sizeof(long),0);
odbieranie wiad -> msgrcv(queue_id, &msg, sizeof(Message) - sizeof(long), -(MAX_MSG_SIZE + 1), 0);
        => type > 0 - wiadomosc o dokladnie danym typie
        => type = 0 - pierwsza wiadomosc w kolejce
        => type < 0 - wiadomosc najmniejszego typu mniejszego od podanej wartości absolutnej


---POSIX---
mqd_t queue;
mqd_t mq_open(const char *name, int oflag [, mode_t mode, struct mq_attr *attr]);
int mq_close(mqd_t mqdes);
int mq_unlink(const char *name);

int mq_send(mqd_t mqdes, const char* ptr, size_t len, unsigned int prio);
ssize_t mq_receive(mqd_t mqdes, char *ptr, size_t len, unsigned int *priop);

-

struct mq_attr attr;
attr.mq_maxmsg = MAX_QUEUE_SIZE;
attr.mq_msgsize = sizeof(Message);
attr.mq_curmsgs = 0;

O_RDWD, O_RDONLU, O_WRONLY
tworzenie -> mqd_t queue_id = mq_open(QUEUE_NAME,O_RDWD | O_CREAT | O_EXCL, 0777, &attr);
odbieranie utworzonej -> mqd_t server_queue_id = mq_open(QUEUE_NAME, O_RDWD);
odlaczenie -> mq_close(queue_id);
usuwanie (tylko raz) -> mq_unlink(QUEUE_NAME);

Message msg;
wysyłanie -> mq_send(queue_id, (char *)&msg, sizeof(Message), PRIORITY);
odbieranie -> mq_receive(queue_id, (char *)&msg,sizeof(Message),NULL);



----------SEMAFORY----------
---SysV---
int semget(key_t key, int nsems, int flag);
int semctl(int semid, int semnum, int cmd, union semun arg);
        => cmd = SETVAL, GETVAL, IPC_RMID
int semop(int semid, struct sembuf *sops, unsigned nsops);

-

key_t key = ftok(home_dir, SEM_KEY);
tworzenie -> int sem = semget(key, 1, IPC_CREAT | IPC_EXCL | 0777);
odbieranie utworzonego -> int sem1 = semget(key, 0 , 0);
ustawianie wartosci -> semctl(sem, 0, SETVAL, val);
pobieranie wartosci -> int value = semctl(sem, 0, GETVAL, 0);
usuwanie -> semctl(sem, 0, IPC_RMID, 0);

-ustawianie wartosci z semun
    union semun arg;
    arg.val = 5;
    semctl(sem, 0, SETVAL, arg);

struct sembuf buffer;
buffer.sem_num = 0;
buffer.sem_flg = 0;

zmniejszanie wartosci ->
                buffer.sem_op = -1;
                semop(sem, &buffer, 1);
zwiekszanie wartosci ->
                buffer.sem_op = 1;
                semop(sem, &buffer, 1);
czekanie na 0 ->
                buffer.sem_op = 0;
                semop(sem, &buffer, 1);


---POSIX---
sem_t semaphore;
sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value); //SEM_FAILED
int sem_close(sem_t *sem);
int sem_unlink(const char *name);
int sem_post(sem_t *sem);
int *sem_wait(sem_t *sem);
int *sem_trywait(sem_t *sem); //EAGAIN
int sem_getvalue(sem_t *sem, int *valp);

-

tworzenie -> sem_t sem = sem_open(SEM_NAME, O_RDWR | O_CREAT | O_EXCL, 0666, val);
odbieranie utworzonego -> sem_t sem1 = sem_open(SEM_NAME, O_RDWR);
pobieranie wartosci -> sem_getvalue(sem,&value); //int value;
odlaczenie -> sem_close(sem);
usuwanie (tylko raz) -> sem_unlink(SEM_NAME);

zmniejszanie wartosci -> sem_wait(sem);
zwiekszanie wartosci -> sem_post(sem);




----------PAMIEC WSPOLDZIELONA----------
---SysV---
int shmget(key_t key, size_t size, int shmflg);
void *shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);
void *shmctl(int shmid, int cmd, struct shmid_ds *buf);

-

key_t key = ftok(home_dir, PROJECT_ID);

Shared_Memory *sh;
tworzenie -> int shared_memory_id = shmget(key, sizeof(struct Shared_Memory) + 10, IPC_CREAT | IPC_EXCL | 0666);
odbieranie utworzonej -> int shared_memory_id = shmget(key, sizeof(struct Shared_Memory) + 10, 0);
dodanie do przestrzeni adr -> sh = shmat(shared_memory_id, 0, 0);
    if (sh == (void *) -1) //blad
odlaczenie od przestrzeni adr -> shmdt(sh);
usuwanie (tylko raz) -> semctl(shared_memory_id, IPC_RMID, 0);



---POSIX---
int shm_open(const char *name, int oflag, mode_t mode);
int ftruncate(int fd, off_t length);
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset); //blad == (void *)-1
int munmap(void *addr, size_t len);
int shm_unlink(const char *name);

-

Shared_Memory *sh;
tworzenie -> int shm_id = shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT | O_EXCL, 0666);
ustawianie rozmiaru (tylko raz) -> ftruncate(shm_id, sizeof(Shared_Memory));
odbieranie utworzonej -> int shm_id1 = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0666);
dodanie do przestrzeni adr -> sh = mmap(NULL,sizeof(Shared_Memory), PROT_READ | PROT_WRITE, MAP_SHARED,shm_id,0);
odlaczenie od przestrzeni adr -> munmap(sh,sizeof(Shared_Memory));
usuwanie (tylko raz) -> shm_unlink(SHARED_MEMORY_NAME);



----------WATKI----------
pthread_t thread;
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
void pthread_exit(void *rval_ptr);
int pthread_join(pthread_t thread, void **rval_ptr);

int pthread_equal(pthread_t tid1, pthread_t tid2);
pthread_t pthread_self(void);
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);

int pthread_cancel(pthread_t tid);
void pthread_testcancel(void);
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_detach(pthread_t tid);

int pthread_sigmask(int how, const sigset_t* set, sigset_t* oset);
int pthread_kill(pthread_t thread, int signo);
int pthread_sigqueue(pthread_t thread, int sig, const union sigval value);

-

funkcja watku -> void *thread_fun(void *arg);
w funkcji ->
    printf("Hello\tTID %ld\targ: %d\n", pthread_self(), *(int*)arg);
    int *res = (int*)arg;
    pthread_exit(res);
    --
    int *tmp = malloc(sizeof(int));
    *tmp = 150;
    pthread_exit(tmp);

tworzenie ->
    int i = 5;
    pthread_t thread;
    int *val = malloc(sizeof(int));
    *val = i;
    pthread_create(&thread, NULL, thread_fun, (void *)val);
    --
    pthread_t *threads;
    threads = malloc(no_threads * sizeof(pthread_t));
    pthread_create(&threads[i], NULL, thread_fun, (void *)val); //val jw


joinowanie ->
    void* retval;
    pthread_join(thread, &retval);
    int *result = (int *)retval;
    printf("retval: %d\n", *result);
    --
    long *time;
    pthread_join(threads, (void *) &time);
    printf("\tVal: %ld\n", *time);

cancel -> CANCEL domyslne ENABLED i DEFFERED
    poza watkiem -> pthread_cancel(thread);
    w watku -> pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE/ENABLED, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED/ASYNCHRONOUS, NULL);
    join => PTHREAD_CANCELED

sprawdzanie watkow ->
    pthread_self();
    pthread_equal(thread1, thread2);



----------MUTEXY----------
pthread_mutex_t mutex;
PTHREAD_MUTEX_INITIALIZER ==>
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex,const struct timespec *restrict abs_timeout);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex); //tylko jak dajemy init

-

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_lock(&mutex);
pthread_mutex_unlock(&mutex);
tylko jak mamy init -> pthread_mutex_destroy(&mutex);


----------SEMAFORY NIENAZWANE----------
sem_t semaphore;
int sem_init(sem_t *sem, int pshared, unsigned int value); //pshared = 0 - miedzy watkami, !=0 - miedzy procesami
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
int sem_post(sem_t *sem);
int sem_destroy(sem_t *sem);



----------ZMIENNE WARUNKOWE/WARUNKI SPRAWDZAJĄCE----------
pthread_cond_t cond;
PTHREAD_COND_INITIALIZER ==>
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *timeout);
int pthread_cond_broadcast(pthread_cond_t *cond); //powiadamia wszystkie watki o zmienie
int pthread_cond_signal(pthread_cond_t *cond); //powiadamia tylko jeden, losowy watek

int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond  = PTHREAD_COND_INITIALIZER;

w jednym watku ->
    pthread_mutex_lock(&mutex);
    while (x <= y) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

w innym ->
    pthread_mutex_lock(&mutex);
    y++;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);



----------SOCKETY----------
UNIX datagram->

--server/sender
     struct sockaddr_un addr;
     addr.sun_family = AF_UNIX;
     sprintf(addr.sun_path, "%s", name);
    //
    struct sockaddr_un addr = {
            .sun_family = AF_UNIX,
            .sun_path = SOCK_PATH
    };

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    bind(fd,(const struct sockaddr *) &addr, sizeof(struct sockaddr_un));


    char buf[20];
    if(read(fd, buf, 20) == -1) //error
    int val = atoi(buf);
    printf("%d square is: %d\n",val,val*val);

    shutdown(fd, SHUT_RDWR);
    close(fd);
    unlink(SOCK_PATH);

--client/receiver
    int fd = -1;

    struct sockaddr_un addr = {
            .sun_family = AF_UNIX,
            .sun_path = SOCK_PATH
    };

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);

    connect(fd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un));

    int to_send = sprintf(buff, argv[1]);

    if(write(fd, buff, to_send+1) == -1) //blad

    shutdown(fd, SHUT_RDWR);
    close(fd);

-------
UNIX stream ->

--server/receiver
    struct sockaddr_un addr = {
            .sun_family = AF_UNIX,
            .sun_path = SOCK_PATH
    };

    char buf[4];
    int s = socket(AF_UNIX, SOCK_STREAM, 0);
    bind(s, (const struct sockaddr *) &addr, sizeof(addr);
    listen(s, 1);
    int fd = accept(s, NULL, 0);
    read(fd, buf, 4);
    printf("%4s\n", buf);

    shutdown(s, SHUT_RDWR);
    close(s);
    unlink(SOCK_PATH);

--client/sender
    struct sockaddr_un addr = {
            .sun_family = AF_UNIX,
            .sun_path = SOCK_PATH
    };
    char buf[4] = "abcd";
    int c = socket(AF_UNIX, SOCK_STREAM, 0);
    connect(c, (const struct sockaddr *) &addr, sizeof(addr);
    write(c, buf, 4);

    shutdown(c, SHUT_RDWR);
    close(c);


--
socket(2) creates a socket,
connect(2) connects a socket to a remote socket address
bind(2) function binds a socket to a local socket address,
listen(2) tells the socket that new connections shall be accepted,
accept(2) is used to get a new socket with a new incoming connection.