#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

struct task{
	int period;
	float utilisation;
	int executionTime;
	int occurence;
	int executing;
	int priority;
}task;

void generateTaskSet(int n, float umax, struct task mySet[]);

int rm_scheduling(struct task mySet[],struct task tmpSet[],struct task svgSet[], int n,int start,int* nb_promotion,int schedule[6][50000]);

int rmpp_scheduling(struct task mySet[],struct task tmpSet[],struct task svgSet[],int start, int promotion, int miss,int n,int nb_promotion,int schedule[6][50000]);

int hyperPeriod (struct task mySet[], int n);

void printSet(struct task mySet[], int n);

void analyse(struct task mySet[], int n);

void displaySchedule(int schedule[6][50000],struct task svgSet[],int n);

void customSet();

int main (void){

    float umax,totalU;
    int n,test=1;
    int i, j,choice;
    srand(time(NULL));

    printf("Do you want to input a task set ? (1/yes   2/no) : ");
    scanf("%d",&choice);

    if(choice==1){
        customSet();
        return 0;
    }

    printf("U_MAX : ");
    scanf("%f",&umax);

    printf("\nNumber of tasks (0 if you want the number of task to be generated randomly between 2 and 10) : ");
    scanf("%d",&n);
    system("cls");

    if(n==0) n=rand()%9+2;

    struct task mySet[n];

    while(test!=0){
        test=0;
        totalU=0;
        generateTaskSet(n,umax,mySet);
        for(i=0;i<n;i++){
            if(mySet[i].executionTime==0) test=1;
        }
        for(i=0;i<n;i++) totalU=totalU+mySet[i].utilisation;
        if(totalU>umax+0.02||totalU<umax-0.02) test=1;
    }
    printf("Total Utilisation = %f \n",totalU);

    for(i=1;i<n;i++){                                   //tri du tableau par priorite (periode)
            for(j=0;j<n-1;j++){
                if(mySet[i].period<mySet[j].period){
                    struct task tmp=mySet[i];
                    mySet[i]=mySet[j];
                    mySet[j]=tmp;
                }
            }
    }

    for(i=0;i<n;i++){
        mySet[i].priority=n-i;
    }

    printSet(mySet,n);
    int hyp=hyperPeriod(mySet,n);
    printf("\nHyperperiod : %d \n\nStart analyse ? (Enter)",hyp);
    getchar();
    getchar();
    analyse(mySet,n);



return 0;
}




void customSet(){
    system("cls");
    int i,j;

    int n;

    printf("How many tasks ? ");
    scanf("%d",&n);
    printf("\n\n");

    struct task mySet[n];

    for(i=0;i<n;i++){
        printf("Task %d\nExecution time : ",i);
        scanf("%d",&mySet[i].executionTime);
        printf("\nPeriod : ");
        scanf("%d",&mySet[i].period);
        printf("\n\n");

        mySet[i].executing=0;
        mySet[i].occurence=0;
        mySet[i].utilisation=(float)mySet[i].executionTime/mySet[i].period;

    }

     for(i=1;i<n;i++){                                   //tri du tableau par priorite (periode)
            for(j=0;j<n-1;j++){
                if(mySet[i].period<mySet[j].period){
                    struct task tmp=mySet[i];
                    mySet[i]=mySet[j];
                    mySet[j]=tmp;
                }
            }
    }

    for(i=0;i<n;i++){
        mySet[i].priority=n-i;
    }

    printSet(mySet,n);
    int hyp=hyperPeriod(mySet,n);
    printf("\nHyperperiod : %d \n\nStart analyse ? (Enter)",hyp);
    getchar();
    getchar();
    analyse(mySet,n);


}




void analyse(struct task mySet[], int n){

    int i;
    int promotion,nb_promotion=0;  //Temps auquel la priorite va augmenter et nombre de taches à promouvoir
    int start=0;    //Temps a partir duquel effectuer rmpp
    int y=1;        //0 si rmp concluant, 1 sinon

    struct task tmpSet[n];      //tmp et mySet vont evoluer jusqua l'hyperPeriod
    struct task svgSet[n];      //svg Permet de conserver le set otiginal, les temps d'execution
    for(i=0;i<n;i++){
        tmpSet[i]=mySet[i];
        svgSet[i]=mySet[i];
    }

    int schedule[6][50000];


    int miss=rm_scheduling(mySet,tmpSet,svgSet,n,start,&nb_promotion,schedule);

    if(miss==0){
        printf("\nThis set is RM schedulable\n");
    }else{
        printf("\nThis set is not RM schedulable\n");
        for(i=0;i<n;i++){
            mySet[i].occurence=0;
            mySet[i].executing=0;
        }
    }

    while(miss!=0){                         //Si le set n'est pas RM schedulable
        promotion=miss;                     //promotion est initialisé a la deadline manquee
        while(promotion!=start&&y==1){
            promotion--;                    //puis decremente jusqua ce que la tache meet sa deadline
            for(i=0;i<n;i++) tmpSet[i]=mySet[i];
            y=rmpp_scheduling(mySet,tmpSet,svgSet,start,promotion,miss,n,nb_promotion,schedule);  //on execute rmpp entre start et miss, en modifiant promotion
        }                                    // Si on parvient à respecter la deadline manquée
        if(y==0){
            nb_promotion=0;
            start=miss+1;
            miss=0;
            miss=rm_scheduling(mySet,tmpSet,svgSet,n,start,&nb_promotion,schedule);      //On repart sur du RM juste apres la deadline.
            if(miss!=0)y=1;
        }else{
            printf("\nThis set is not RM-PP schedulable\n");
            break;
        }
    }

    int totalPromo=0;
    int hyp=hyperPeriod(svgSet,n);
    for(i=1;i<hyp;i++){
        if((schedule[0][i]==1&&schedule[0][i-1]==0)||(schedule[0][i]==1&&schedule[2][i]!=schedule[2][i-1]))totalPromo++;
    }

    if(y==0)printf("\nThis set is RM-PP schedulable\n\nPromotions : %d\n",totalPromo);

    printf("\n\nDo you want to display the schedule (1/yes 2/no) ? ");
    scanf("%d",&i);

    if(i==1) displaySchedule(schedule,svgSet,n);

}



int hyperPeriod (struct task mySet[], int n){

    int x=0,i,j=0,c;

    while(x==0){
        j++;
        c=0;
        for(i=0;i<n;i++){
            if(j%mySet[i].period==0) c++;
        }
        if(c==n) x=j;
    }
    j=0;
    return x;

}


void generateTaskSet(int n, float umax,struct task mySet[]){


    float sumU=umax;
    float totalU=0;

    int i;
	for(i=1; i<=n-1; i++){
        float tmp = pow((rand()/(double)RAND_MAX ) , (1/(float)(n-i)));
		float nextSumU = sumU*tmp;
		mySet[i-1].utilisation = sumU-nextSumU;
		mySet[i-1].period=rand()%41+10;
		mySet[i-1].executionTime=mySet[i-1].utilisation*mySet[i-1].period;
		mySet[i-1].utilisation=(float)mySet[i-1].executionTime/mySet[i-1].period;
		mySet[i-1].occurence=0;
		mySet[i-1].executing=0;
		sumU = nextSumU;
	}
	for(i=0;i<n-1;i++) totalU=totalU+mySet[i].utilisation;
    mySet[n-1].utilisation=umax-totalU;
    mySet[n-1].period=rand()%41+10;
    mySet[n-1].executionTime=mySet[n-1].utilisation*mySet[n-1].period;
    mySet[n-1].utilisation=(float)mySet[n-1].executionTime/mySet[n-1].period;
    mySet[n-1].occurence=0;
    mySet[n-1].executing=0;

}


void printSet(struct task mySet[], int n){

    int i;
    for(i=0;i<n;i++){
        printf("task : %d\n   e=%d   T=%d   U=%f   oc=%d   exc=%d\n",i,mySet[i].executionTime,mySet[i].period,mySet[i].utilisation,mySet[i].occurence,mySet[i].executing);
    }

}


void displaySchedule(int schedule[6][50000],struct task svgSet[],int n){
    int j;
    int hyp=hyperPeriod(svgSet,n);
    system("cls");

    printf("promotion   time   task   instance   deadline   execution left\n");
    for(j=0;j<hyp;j++){
        printf("    %d        %d      %d        %d         %d        %d\n",schedule[0][j],schedule[1][j],schedule[2][j],schedule[3][j],schedule[4][j],schedule[5][j]);
    }


}


int rm_scheduling(struct task mySet[],struct task tmpSet[],struct task svgSet[], int n,int start,int* nb_promotion,int schedule[6][50000]){

    int miss=0;
    int i,c=0;      //C est egal au nombre de taches qui ratent leur deadline

    int ex;
    int t=0;
    int hyp=hyperPeriod(svgSet,n);
    for(t=start;t<hyp;t++){     //Le temps défile

        for(i=0;i<n;i++){                       //On regarde si on atteint une periode
            if(t%svgSet[i].period==0){
                tmpSet[i].executing=1;
                if(tmpSet[i].executionTime!=0&&t!=0){
                    c++;                   //Si on rate une deadline, on incremente c.
                }
                if(c!=0){
                    *nb_promotion=c;        //Si c!=0, on met à jour le nombre de taches à promouvoir et on renvoie la deadline manquée
                    return t;
                }
                else tmpSet[i].executionTime=svgSet[i].executionTime;
            }
        }

        ex=-1;
        int highest=-1;
        for(i=0;i<n;i++){
            if(tmpSet[i].executing==1){
                highest=i;
                break;
            }
        }
        for(i=1;i<n;i++){                       //On selectionne la tache executable avec la plus haute priorité
            if(tmpSet[i].executing&&(tmpSet[i].priority>tmpSet[highest].priority)){
                highest=i;
            }
        }
        ex=highest;

        if(t<50000){
        schedule[0][t]=0;
        schedule[1][t]=t;
        schedule[2][t]=ex;
        schedule[3][t]=tmpSet[ex].occurence+1;
        schedule[4][t]=tmpSet[ex].period*(tmpSet[ex].occurence+1);
        schedule[5][t]=tmpSet[ex].executionTime-1;}

        if(ex!=-1){                                                            //On fait diminuer le temps necessaire restant pour loccurence
            if(tmpSet[ex].executionTime>0){
                tmpSet[ex].executionTime=tmpSet[ex].executionTime-1;

            }
            if(tmpSet[ex].executionTime==0){
                tmpSet[ex].occurence++;                                  //On change doccurence
                tmpSet[ex].executing=0;                                  //La tache nest plus executable
            }
        }


    }

    return miss;

}



int rmpp_scheduling(struct task mySet[],struct task tmpSet[],struct task svgSet[],int start, int promotion, int miss,int n,int nb_promotion,int schedule[6][50000]){

    int i,j;
    int ex;
    int t=0;

    for(t=start;t<=miss;t++){     //Le temps défile

        for(i=0;i<n;i++){                       //On regarde si on atteint une periode
            if(t%svgSet[i].period==0){
                tmpSet[i].executing=1;
                if(tmpSet[i].executionTime!=0&&t!=0) return 1;      //si une tache rate sa deadline, fini, c'est pas RM-PP schedulable
                else tmpSet[i].executionTime=svgSet[i].executionTime;
            }
        }

        if(t==promotion){       //Si on atteint le moment de promotion, on regarde quelles task doivent etre promues
            for(i=n-1;i>=n-nb_promotion;i--){
                tmpSet[i].priority=50;
                for(j=0;j<n;j++){
                    if(tmpSet[j].period==tmpSet[i].period) tmpSet[j].priority=50;
                }
            }
        }
        if(t==miss){       //une fois que c'est passe on remet les priorites
            for(i=0;i<n;i++){
                tmpSet[i].priority=svgSet[i].priority;
            }
        }


        ex=-1;
        int highest=-1;
        for(i=0;i<n;i++){
            if(tmpSet[i].executing==1){
                highest=i;
                break;
            }
        }
        for(i=1;i<n;i++){                       //On selectionne la tache executable avec la plus haute priorité
            if(tmpSet[i].executing&&(tmpSet[i].priority>tmpSet[highest].priority)){
                highest=i;
            }
        }
        ex=highest;

    if(t<50000){
        if(tmpSet[ex].priority==50)schedule[0][t]=1;
        else schedule[0][t]=0;
        schedule[1][t]=t;
        schedule[2][t]=ex;
        schedule[3][t]=tmpSet[ex].occurence+1;
        schedule[4][t]=tmpSet[ex].period*(tmpSet[ex].occurence+1);
        schedule[5][t]=tmpSet[ex].executionTime-1;
    }


        if(ex!=-1){                                                            //On fait diminuer le temps necessaire restant pour loccurence
            if(tmpSet[ex].executionTime>0){
                tmpSet[ex].executionTime=tmpSet[ex].executionTime-1;

            }
            if(tmpSet[ex].executionTime==0){
                tmpSet[ex].priority=svgSet[ex].priority;
                tmpSet[ex].occurence++;                                  //On change doccurence
                tmpSet[ex].executing=0;                                  //La tache nest plus executable
            }
        }
    }

    for(i=0;i<n;i++) mySet[i]=tmpSet[i];

    return 0;

}





