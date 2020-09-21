#include "sema.h"
#include <sys/shm.h>
#include <time.h>

#define MAX_PLAYERS	100		// numero de jogadores apenas
#define MAX_BOLAS 	100		//numero máximo das bolas
#define MAX_CHILD	100 		// numero máximo de processos(jogadores mais arbitros etc)
#define LIFETIME	30		/* 30 minutos de jogo */
#define SHMKEY (key_t) 0x10		//chave dos jogadores
#define SHMKEY1 (key_t) 0x11		//chave dos golos
#define SHMKEY2 (key_t) 0x12		//chave dos remates
#define SHMKEY3 (key_t) 0x13		//chave das faltas


int nrJogadores;	// variável onde se vai guardar o número de jogadores que o utilizador insere
int nrBolas;		// variável onde se vai guardar o número de bolas que o utilizador insere
int nrProcessos;
semaphore mutex;	// semáforo mutex que restringe o acesso à memória partilhada
int *jogadores;		// guarda o numero de bolas de cada jogador, cada jogador será index do array
int *golos;		// guarda os golos de cada jogador
int *remates;		// guarda os remates de cada jogador
int *faltas;		// guarda as faltas feitas por cada jogador

semaphore arraySema[MAX_PLAYERS]; //array que contem semaforos dos jogadores


int encontraAdversario(int jogadorAtual){  // método que gera aleatóriamente um adversário tendo em conta que não pode escolher o jogador atual
	int adv;
	
	do{
		adv  =rand() % nrJogadores;
	}while(adv == jogadorAtual);

    return adv;
}

void rematar(int nrJogador, int jogadorAdv){  // método que recebe o jogador que vai rematar e o jogador que vai tentar defender,
					      // faz todas as operações referentes ao remate e decide se marcou ou não

    printf("O jogador %d recebeu uma bola e vai rematar contra o jogador %d\n",nrJogador+1,jogadorAdv+1) ;
    int r = rand() % 2;

    *(jogadores+nrJogador) = *(jogadores+nrJogador)-1;
    *(jogadores+jogadorAdv) = *(jogadores+jogadorAdv)+1;
	*(remates+nrJogador) = *(remates+nrJogador)+1;

    if(r==0){
		printf("Marcou.\n");
		*(golos+nrJogador) = *(golos+nrJogador)+1;
    }
    if(r==1) printf("Defendeu.\n");
       
}	
                    
			

void jogador(int i){  // método que contem todo o código que um processo deve fazer quando se trata de um jogador, recebe o número do jogador como argumento

    P(arraySema[i]);
	// verifica se o jogador comete falta
	int tempo = rand() % 601; 
                    if (tempo > 500) {
			
                    *(faltas + i) = *(faltas + i) + 1;
                    printf("Jogador %d cometeu uma falta \n",i+1);
                    tempo = 500;
			
                    }


                    usleep(tempo * 1000);

	int jogadorAdv = encontraAdversario(i);
	P(mutex);
	rematar(i, jogadorAdv);
	V(mutex);
	V(arraySema[jogadorAdv]);

}


void atribuirBolas(){  //método que atribui todas as bolas aos jogadores

	int count =0;
	int jogador=0;

	do{

		jogador = rand() % nrJogadores;

		*(jogadores+ jogador)= *(jogadores+ jogador)+1;
			
		if(*(jogadores+ jogador) >= 2){
			*(jogadores+ jogador)= *(jogadores+ jogador)-1;
		}else{
			count= count+1;
			                
			V(arraySema[jogador]);
		}
	}while(count != nrBolas);
}


void iniciarSemaforos(){  //método que inicia todos os semáforos dos jogadores, no arraySema[i]
    int i;
	for(i =0 ; i<nrJogadores; i++){
		arraySema[i] = init_sem(0);
	}
	
}

void shellSort(int *vet, int size, int *jogador) { //método que ordena pelo numero de jogadores e de acordo com a sua identificação
    int i , j , value, value2;
    int gap = 1;
    while(gap < size) {
        gap = 3*gap+1;
    }
    while ( gap > 1) {
        gap /= 3;
        for(i = gap; i < size; i++) {
            value = vet[i];
			value2 = jogador[i];
			j = i - gap;
            while (j >= 0 && value < vet[j]) {
                vet [j + gap] = vet[j];
				jogador[j + gap] = jogador[j];
                j -= gap;
            }
            vet [j + gap] = value;
			jogador[j+gap]= value2;
        }
    }
}

int isProlongamento(){	//método que verifica se existe ou não prolongamento
	int arrayGolos[nrJogadores];				//vai guardar os golos de cada jogador
	int arrayNumJogadores[nrJogadores];			//vai guardar o numero do jogador (identificação)
	int i = 0;	
	for( ; i<nrJogadores; i++){
		
		arrayNumJogadores[i]= i+1;
		arrayGolos[i]=*(golos+i);
	}

	shellSort(arrayGolos,nrJogadores,arrayNumJogadores);	
	i = nrJogadores-1;
	int maiorNumGolos=arrayGolos[i];
	int count = 0;
	for(;i>-1;i--){
		if(maiorNumGolos == arrayGolos[i]){
			count++;
		}
	}
	
	if(count>1){
		return 1;
	}else{
		return 0;
	}
}

void podio(){  //método que mostra na consola as classificações dos jogadores e o n+umero de golos marcados por ordem
	int array[nrJogadores];				//vai guardar os golos de cada jogador
	int arrayNumJogadores[nrJogadores];		//vai guardar o numero do jogador (identificação)
	int i=0;	
	for( ; i<nrJogadores; i++){
		
		arrayNumJogadores[i]= i+1;
		array[i]=*(golos+i);
	}

	shellSort(array,nrJogadores,arrayNumJogadores);	
	i=nrJogadores-1;
	for(;i>-1;i--){
		printf("O jogador que está em %d lugar é o jogador %d, com %d golos marcados.\n", nrJogadores - i, arrayNumJogadores[i], array[i]);
	}
}

void nrDeRemates(){  //método que mostra na consola o número de remates de cada jogador
	int i =0;
	for(;i<nrJogadores;i++){
		printf("O jogador %d rematou %d vezes.\n", i+1, *(remates + i));
	}
}

void precisaoRemates(){ //método que mostra na consola a percentagem da precisão de remates de todos os jogadores
	int i =0;
	for(;i<nrJogadores;i++){
		if(*(remates+i) == 0){
		printf("O jogador %d não chegou a rematar.\n", i+1 );
		}else{
		float precisao = (float)  *(golos + i)/ *(remates + i);
		int percentagem = (int)(precisao * 100);
		printf("A precisão do jogador %d foi de %d%c \n", i+1,percentagem , '%');
		}
	}
}
void nrFaltas(){  //método que mostra na consola o número de faltas de cada jogador
	int i =0;
	for(;i<nrJogadores;i++){
		printf("O jogador %d fez %d faltas.\n", i+1, *(faltas + i));
	}
}

int bolasAtribuidas(){
	int count =0;
	int i=0;
	for(;i<nrJogadores;i++){
		if(*(jogadores + i) == 1){
			count++;
		}
	}
	if(count==nrBolas){
		return 1;
	}else{
		return 0;
	}
}

main ()
{       
	printf("Insira o número de jogadores (não deverá ser maior que 100): ");
	scanf("%d", &nrJogadores);
        
	nrProcessos = nrJogadores;

	printf("Insira o número de bolas (não deverá ser maior que %d): ",nrJogadores);
	scanf("%d", &nrBolas);

	/* espaço de memória partilhada do número de bolas dos jogadores */
	int shmid = shmget(SHMKEY, nrJogadores, 0777|IPC_CREAT);
	char *addr = (char *) shmat(shmid,0,0);
	jogadores = (int*) addr;

	/* espaço de memória partilhada do número de golos dos jogadores */
	int shmid1 = shmget(SHMKEY1, nrJogadores, 0777|IPC_CREAT);
	char *addr1 = (char *) shmat(shmid1,0,0);
	golos = (int*) addr1;

	/* espaço de memória partilhada do número de remates dos jogadores */
	int shmid2 = shmget(SHMKEY2, nrJogadores, 0777|IPC_CREAT);
	char *addr2 = (char *) shmat(shmid2,0,0);
	remates = (int*) addr2;

	/* espaço de memória partilhada do número de faltas dos jogadores */
	int shmid3 = shmget(SHMKEY3, nrJogadores, 0777|IPC_CREAT);
	char *addr3 = (char *) shmat(shmid3,0,0);
	faltas = (int*) addr3;

	int		child_pid [MAX_CHILD],			/* process ID's */
		wait_pid;					/* pid of terminated child */
	int		i, j,					/* loop variables */
		child_status;					/* return status of child */
			
	mutex = init_sem(1);

	iniciarSemaforos();
	
		
	for (i = 0; i < nrProcessos; ++i)
	{
		child_pid [i] = fork ();
                
		switch (child_pid [i])
		{
			case -1:			/* error: no process created	*/
				perror ("fork failed");
				exit (1);
				break;
			case 0:				/* child process		*/
				alarm(LIFETIME);
	            
				for(;;)
	            {
	            
	            if(i<nrJogadores) jogador(i);

	            }
	            break;
			default:			/* parent process		*/
				if(bolasAtribuidas() == 0){
				srand((int) time(NULL));
				atribuirBolas();
				}

				if (i == (nrProcessos - 1))/* all childs created ?	*/
				{			/* yes -> wait for termination	*/
					for (j = 0; j < nrProcessos; ++j)
					{
						wait_pid = wait (&child_status);
						if (wait_pid == -1)
						{
							perror ("wait failed");
						};
					};

					puts("\nO Jogo acabou.\n");

					if(isProlongamento() == 1) puts("Haveria prolongamento\n");

					puts("------PODIO------");
					podio();

					puts("\n------NUMERO DE REMATES------");
					nrDeRemates();

					puts("\n------PERCENTAGEM DE PRECISAO DOS REMATES------");
					precisaoRemates();
	
					puts("\n------NUMERO DE FALTAS------");
					nrFaltas();

					rel_sem (mutex);
					
					// libertação de todos os semáforos referentes aos jogadores
					int i=0;
					for( ; i<nrJogadores; i++){
                        		rel_sem(arraySema[i]);
                    			}

					// libertação das memórias partilhadas
					shmdt(addr);
					shmdt(addr1);
					shmdt(addr2);
					shmdt(addr3);
					shmctl(shmid, IPC_RMID, NULL);
					shmctl(shmid1, IPC_RMID, NULL);
					shmctl(shmid2, IPC_RMID, NULL);
					shmctl(shmid3, IPC_RMID, NULL);

			printf("\nDeseja recomeçar o jogo?\n1-Sim\n2-Não\nR: ");
			int resposta =0;
			scanf("%d", &resposta);
			if(resposta==1) main();

			}
		}					/* end switch			*/
	}					/* end for			*/
}
