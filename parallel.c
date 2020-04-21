#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <sched.h>
#include <pthread.h>

int thread_num=1;
char template_cmd[4096];
int template_len;
void *input_lock;

int UToA(char *str,int num){
	static char buf[12];
	char *pt=buf;
	for(; num; num/=10) *pt++=num%10+'0';
	int len=pt-buf;
	while(pt!=buf) *str++=*--pt;
	return len;
}

void PrintHelp(char *self_name){
	assert(self_name);
	printf("Usage: %s [flags] \"command\"\n",self_name);
	printf("Sample: seq 1 100 | %s -j 2 \"echo {} \"\n",self_name);
	printf("\n");
	printf("Flags:\n");
	printf("  -j, --jobs int          run n jobs in parallel (default 1)\n");
	printf("\n");
	printf("Patterns supported:\n");
	printf("  {} {.} {#} {##}\n");
}

int ParseArg(int argc,char **argv){
	for(int i=1; i<argc; ++i){
		char *arg=argv[i];
		if(!strcmp(arg,"-h")||!strcmp(arg,"--help")){
			PrintHelp(argv[0]);
			return 0;
		} else if(arg[0]!='-'){
			strcpy(template_cmd,arg);
			template_len=strlen(arg);
		} else if(i==argc-1){
			fprintf(stderr,"Error: Invalid argument / Missing value: %s\n",arg);
			PrintHelp(argv[0]);
			return 0;
		} else if(!strcmp(arg,"-j")){
			thread_num=atoi(argv[++i]);
			if(thread_num<=0||thread_num>256){
				fprintf(stderr,"Please choose a suitable number of threads(>=1,<=256).\n");
				return 0;
			}
		}
	}
	if(!template_len){
		fprintf(stderr,"Please type your command\n");
		return 0;
	}
	return 1;
}

int GetCurArg(char *str){
	//TODO: support ::: patterns
	return !fgets(str,4096,stdin);
}

void* Worker(void *arg){
	static int task_id=0;
	char *buf=malloc(4096*sizeof(char)),*real_cmd=malloc(4096*sizeof(char));
	const int thread_id=*(int*)arg;
	for(;;){
		while(__atomic_test_and_set(input_lock,0)) sched_yield();
		if(GetCurArg(buf)){
			__atomic_clear(input_lock,0);
			return 0;
		}
		__atomic_clear(input_lock,0);
		int cur_id=++task_id;
		int buf_len=strlen(buf);
		//TODO: rename variables
		for(char *tgt=real_cmd,*lst=template_cmd,*pos=strchr(lst,'{');;){
			strncpy(tgt,lst,pos-lst);
			tgt+=pos-lst;
			if(*pos=='\0'){
				*tgt++='\0';
				break;
			}
			if(*++pos=='}'){
				strcpy(tgt,buf);
				tgt+=buf_len;
			} else if(*pos=='.'){
				assert(*++pos=='}');
				char *pt=strrchr(buf,'.');
				if(pt==0) pt=buf+buf_len;
				strncpy(tgt,buf,pt-buf);
				tgt+=pt-buf;
			} else if(*pos=='#'){
				if(*++pos=='}') tgt+=UToA(tgt,cur_id);
				else if(*pos=='#'){
					assert(*++pos=='}');
					tgt+=UToA(tgt,thread_id);
				}
				else{
					fprintf(stderr,"[Error] does not support {#%c} at the moment.\n",*pos);
					exit(0);
				}
			} else{
				fprintf(stderr,"[Error] does not support {%c} at the moment.",*pos);
				exit(0);
			}
			if(*++pos=='\0'){
				*tgt++='\0';
				break;
			}
			lst=pos;
			pos=strchr(pos,'{');
			pos=pos==0?template_cmd+template_len:pos;
		}
		assert(!system(real_cmd));
	}
	return 0;
}

int main(int argc,char **argv){
	if(!ParseArg(argc,argv)) return 0;
	input_lock=malloc(sizeof(void*));
	pthread_t *threads=malloc(thread_num*sizeof(pthread_t));
	int *thread_arg=malloc(thread_num*sizeof(int));
	for(int i=0; i<thread_num; ++i){
		*(thread_arg+i)=i+1;
		assert(!pthread_create(threads+i,0,Worker,thread_arg+i));
	}
	for(int i=0; i<thread_num; ++i){
		int *thread_ret=0;
		assert(!pthread_join(threads[i],(void**)&thread_ret));
	}
	return 0;
}
