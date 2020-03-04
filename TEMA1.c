//PETRUSCA BOGODAN-MIHAI
//313CB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
typedef struct history
{
    char *string;
    int transfer_order;
    struct history *urm;
}THistory, *LHistory, **ALHistory;
 
typedef struct card_list
{
    char card_nr[17],expiry_date[6], status[8], pin[5], CVV[4];
    int balance, nr_incercari_pin;
    LHistory history;
    struct card_list *urm;
}TCardList, *LSC, **ALSC;
   
 
typedef struct LC_list
{
    LSC card_list;
    int poz;
    struct LC_list *urm;
}TLC_list, *LC, **ALC;
 
void freeHistory(ALHistory aL)
{
    LHistory aux;
    while(*aL)
    {
        aux = *aL;
        *aL = aux->urm;
        free(aux->string);
        free(aux);
    }
}
 
void freeLSC(ALSC aL)
{
    LSC aux;
    while(*aL)
    {
        aux = *aL;
        *aL = aux->urm;
        if(aux->history != NULL)
        freeHistory(&aux->history);
        free(aux);
    }
}
 
void freeLC(ALC aL)
{
    LC aux;
    while(*aL)
    {
        aux = *aL;
        *aL = aux->urm;
        freeLSC(&aux->card_list);
        free(aux);
    }
}
 
LHistory AlocareHistory()
{
    LHistory aux;
    aux = (LHistory) malloc(sizeof(THistory));
    aux -> string = NULL;
    aux->transfer_order = 0;
    if(!aux) return NULL;
    aux->urm = NULL;
    return aux;
}
 
LSC AlocareLSC()
{
    LSC aux;
    aux = (LSC) malloc(sizeof(TCardList));
    if(!aux) return NULL;
    aux->nr_incercari_pin = 0;
    aux->balance = 0;
    aux->history = AlocareHistory();
    if(!aux->history) {freeLSC(&aux); return NULL;}
    aux->urm = NULL;
    return aux;
}
 
LC AlocareLC()
{
    LC aux;
    aux = (LC) malloc(sizeof(TLC_list));
    if(!aux) return NULL;
    aux->card_list = NULL;
    aux->urm = NULL;
    aux->poz = 0;
    return aux;
}
// functia aceasta va intoarce un numar ce reprezinta numarul celulei de LC
//in care se afla lista de carduri in care voi adauga cardul
int calc_poz(long long a,int b)
{
    int s = 0;
    while(a) { s += a % 10; a /= 10; }
    return s % b;
}
 //functia aceasta va insera un card la inceputul listei LSC
void InsInc(LSC *L_cards, char *card_nr, char *pin, char *expiry_date, char *CVV)
{
	//alocam si copiem valorile
    LSC aux = AlocareLSC();
    if(!aux) return;
    strcpy(aux->card_nr, card_nr);
    strcpy(aux->pin, pin);
    strcpy(aux->expiry_date, expiry_date);
    strcpy(aux->CVV, CVV);
    strcpy(aux->status, "NEW");
    //cream legaturile dinte aux si lista
    aux->urm = *L_cards;
    *L_cards = aux;
}
//functia aceasta va insera un card la inceputul listei LSC.
//diferenta dintre aceasta si cea de mai sus este ca pe aceasta
//o folosim atunci cand trebuie sa facem resize
void aux_InsInc(LSC *L_cards, char *card_nr, char *pin, char *expiry_date, char *CVV, LHistory history, char *status, int balance, int nr_incercari_pin)
{
    LSC aux = AlocareLSC();
    if(!aux) return;
    strcpy(aux->card_nr, card_nr);
    strcpy(aux->pin, pin);
    strcpy(aux->expiry_date, expiry_date);
    strcpy(aux->CVV, CVV);
    strcpy(aux->status, status);
    THistory aux_i = *history;
    memcpy(aux->history, &aux_i, sizeof(THistory));
    aux->nr_incercari_pin = nr_incercari_pin;
    aux->balance = balance;
    aux->urm = *L_cards;
    *L_cards = aux;
}
 
 //functia aceasta va adauga la sfarsitul listei LC o noua celula
void InsSf(LC *aL, LC aux)
{
    LC u = NULL, p = *aL;
    if(p != NULL)
    {
        while(p->urm)
            p = p->urm;
        u = p;
    }
    if(u == NULL)
        *aL = aux;
    else
        u->urm = aux;
 
}

//functia aceasta cauta un card si il elimina din LSC
void Eliminare(ALSC aL, char *card_nr)
{
    LSC ant, p;
 
    for(p = *aL, ant = NULL; p != NULL; ant = p, p = p->urm)
    {
        if(strcmp(p->card_nr,card_nr) == 0)
        {
            break;
        }
    }
    if(p == NULL)
        return;
    if(ant == NULL)
        *aL = p->urm;
    else
        ant->urm = p->urm;
    free(p->history);
    free(p);
}
 
//functia aceasta parcurge lista de LSC-uri, iar apoi se 
//apeleaza functia de mai sus pentru a elimina cardul dorit
void delete_card(LC *L, char *card_nr)
{
    LC p = *L;
    for(; p != NULL; p = p->urm)
        Eliminare(&p->card_list, card_nr);
}

//functia aceasta verifica daca exista cardul dorit in baza de date
int exista_card (char *card_nr, LC L) {
    LC q;
    for(q = L; q != NULL; q = q->urm)
    {
        LSC list = q->card_list;
        for(; list != NULL; list = list->urm)
            if(strcmp(list->card_nr, card_nr) == 0)
            {
                return 0;
            }
    }
    return 1;
}
//functia aceasta adauga un card in baza de date, pe pozitia dorita
int add_card(LC *L,int nr_max_card, char *card_nr, char *pin, char *expiry_date, char *CVV, FILE *f)
{
	//mai jos verific daca pinul contine litere
	int verif = 0;
    if(pin[0] > 57 || pin[1] > 57 || pin[2] > 57 || pin[3] > 57)
        verif = 1;
    //calculez pozitia
    int poz = calc_poz(atoll(card_nr), nr_max_card), cont = 0;
    LC p = *L, q = *L;
    // in cazul in care pinul nu are 4 caractere de tip cifra afisez mesajul
    if(strlen(pin) < 4 || strlen(pin) > 4 || verif == 1)
    {
        fprintf(f, "Invalid PIN\n");
        return -1;
    }
    //cu ajutorul buclei de mai jos adaug poz celule de LC 
    while(cont <= poz)
    {
        if(p == NULL)
        {
            LC aux = AlocareLC();
            if(!aux) return 0;
            InsSf(L,aux);
        }
        else
            p = p->urm;
        cont++;
    }
    cont = 0;
    //mai jos adaug un nou card pe pozitia ceruta
    for(; q != NULL; q = q->urm)
    {
        if(cont == poz)
        {
            InsInc(&q->card_list,card_nr,pin,expiry_date,CVV);
        }
        cont++;
    }
    return 1;
}
//functia de mai jos are aceeasi functionalitate cu cea de mai sus
//aceasta este folosita in functia de resize, ea adauga toate campurile
//unui card, in comparatie cu cea de mai sus care adauga doar campurile 
//cerute ca parametru de functia "add_card"
void aux_add_card(LC *L,int nr_max_card, char *card_nr, char *pin, char *expiry_date, char *CVV,
 LHistory history, char *status, int balance, int nr_incercari_pin)
{
    int poz = calc_poz(atoll(card_nr), nr_max_card), cont = 0;
    LC p = *L, q = *L;
   
    while(cont <= poz)
    {
        if(p == NULL)
        {
            LC aux = AlocareLC();
            if(!aux) return;
            InsSf(L,aux);
        }
        else
            p = p->urm;
        cont++;
    }
    cont = 0;
    for(; q != NULL; q = q->urm)
    {
        if(cont == poz)
        {
            aux_InsInc(&q->card_list,card_nr,pin,expiry_date,CVV, history, status, balance, nr_incercari_pin);
        }
        cont++;
    }
}
 
//functia de mai jos este folosita pentru a afisa un card anume
void show_card(LC L, char *card_nr, FILE *f)
{
    LC Q = L;
    //caut cardul in baza de date
    for(; Q != NULL; Q = Q->urm)
    {
 
        LSC p = Q->card_list;
        for(; p != NULL; p = p->urm)
            if(!strcmp(p->card_nr, card_nr))
            {
                fprintf(f, "(card number: %s, PIN: %s, expiry date: %s, CVV: %s, balance: %d, status: %s, ",
                    p->card_nr, p->pin, p->expiry_date, p->CVV,
                    p->balance, p->status);
                LHistory history = p->history;
                fprintf(f, "history: [" );
            for(; history->urm != NULL && history->string != NULL; history = history->urm)
            {
               
                fprintf(f, "(%s)", history->string);
                if(history->urm->string != NULL)
                    fprintf(f, ", " );
            }
            fprintf(f, "])");
            }
 
    }
    fprintf(f, "\n");
}
//functia de mai jos este folosita pentru a afisa baza de date
void show(LC L, FILE *f)
{  
    int i = 0;
    LC Q = L;
    for(; Q != NULL; Q = Q->urm)
    {
        LSC p = Q->card_list;
        fprintf(f, "pos%d: [",i++);
        for(; p != NULL; p = p->urm)
        {
            fprintf(f, "\n(card number: %s, PIN: %s, expiry date: %s, CVV: %s, balance: %d, status: %s, ",
                p->card_nr, p->pin, p->expiry_date, p->CVV,
                p->balance, p->status);
            LHistory history = p->history;
            fprintf(f, "history: [" );
            for(; history->urm != NULL && history->string != NULL; history = history->urm)
            {
               
                fprintf(f, "(%s)", history->string);
                if(history->urm->string != NULL)
                    fprintf(f, ", " );
            }
            fprintf(f, "])");
            if(p->urm == NULL)
                fprintf(f, "\n");
 
        }
        fprintf(f, "]\n");
    }
}

//functia de mai jos este folostia pentru a adauga o celula de istoric la inceputul listei aferente
void InsHistory(LHistory *L_history, char *str)
{
    LHistory aux = AlocareHistory();
    if(!aux) return;
    aux->string = strdup(str);
    aux->urm = *L_history;
    *L_history = aux;
}

//functia de mai jos adauga o anumita suma pe cardul dorit
void recharge(LC L, char *card_nr, char *sum, FILE *f)
{
    char string[100];
    //caut cardul
    for(; L != NULL; L = L->urm)
    {
        LSC aux = L->card_list;
        for(; aux != NULL; aux = aux->urm)
        {
            if(strcmp(aux->card_nr, card_nr) == 0)
            {
            	//verific daca suma este divizibila cu 10
            	//copiez mesajul ce trebuie afisat intr-un string, iar apoi
            	//adauga la inceput celula de istoric
                if(atoi(sum) % 10 != 0)
                {
                    fprintf(f, "The added amount must be multiple of 10\n");
                    sprintf(string, "FAIL, recharge %s %s", card_nr, sum);
                    InsHistory(&aux->history, string);
                    return;
                }
                else
                {
                    sprintf(string, "SUCCESS, recharge %s %s", aux->card_nr, sum);
                    InsHistory(&aux->history, string);
                    aux->balance += atoi(sum);
                    fprintf(f, "%d\n",aux->balance );
                }
            }
        }
    }
}
//functia aceasta scoate o anumita suma de bani din balance-ul
//cardului respectiv
void cash_withdrawal(LC L,char *card_nr, char *sum, FILE *f)
{
    char string[100];
   
   //caut cardul in baza de date
    for(; L != NULL; L = L->urm)
    {
        LSC aux = L->card_list;
        for(; aux != NULL; aux = aux->urm)
        {
            if(strcmp(aux->card_nr, card_nr) == 0)
            {
            	// verific daca suma este divizibila cu 10
                if(atoi(sum) % 10 != 0)
                {
                    fprintf(f, "The requested amount must be multiple of 10\n");
                    sprintf(string, "FAIL, cash_withdrawal %s %s", card_nr, sum);
                    InsHistory(&aux->history, string);
                    return;
                }
                //daca cardul nu are destui bani in cont, se va afisa mesajul cerut
                //iar operatia se va adauga in intoric
                if(aux->balance < atoi(sum))
                {
                    fprintf(f, "Insufficient funds\n" );
                    sprintf(string, "FAIL, cash_withdrawal %s %s", card_nr, sum);
                    InsHistory(&aux->history, string);
                }
                //in caz contrar se extrage suma de bani ceruta
            	//se afiseaza balance-ul cardului, si se adauga operatia in istoric
                else
                {
                    aux->balance -= atoi(sum);
                    sprintf(string, "SUCCESS, cash_withdrawal %s %d", aux->card_nr, atoi(sum));
                    InsHistory(&aux->history, string);
                    fprintf(f, "%d\n",aux->balance );
 
                }
            }
        }
    }
}
//functia de mai jos transfera o suma de bani de la un card la altul
void transfer_funds(LC L, char *card_nr_source, char *card_nr_dest, char *sum, FILE *f)
{
    char string[100];
    static int nr = 0;
    //caut sursa si destinatie si ii salvez in 2 pointeri
    LSC source, dest;
    for(; L != NULL; L = L->urm)
    {
        LSC aux = L->card_list;
        for(; aux != NULL; aux = aux->urm)
        {
            if(strcmp(aux->card_nr, card_nr_source) == 0)
                source = aux;
            if(strcmp(aux->card_nr, card_nr_dest) == 0)
                dest = aux;
        }
    }
    //verific daca suma este divizibila cu 10
    if(atoi(sum) % 10 != 0)
    {
        fprintf(f, "The transferred amount must be multiple of 10\n");
        sprintf(string, "FAIL, transfer_funds %s %s %s", card_nr_source, card_nr_dest, sum);
        InsHistory(&source->history, string);
        return;
    }
    //verific daca sursa are destui bani pe card pentru a se efectua tranzactia
    if(source->balance < atoi(sum))
    {
        fprintf(f, "Insufficient funds\n");
        sprintf(string, "FAIL, transfer_funds %s %s %s", card_nr_source, card_nr_dest, sum);
        InsHistory(&source->history, string);
    }
    else
    {
    	//cu ajutorul variabilei nr retin ordinea trasferurilor
    	//aceasta va fi de folos in functie "reverse_transaction"
        source->balance -= atoi(sum);
        dest->balance += atoi(sum);
        fprintf(f, "%d\n",source->balance );
        sprintf(string, "SUCCESS, transfer_funds %s %s %s", card_nr_source, card_nr_dest, sum);
        InsHistory(&source->history, string);
        InsHistory(&dest->history, string);
        nr++;
        dest->history->transfer_order = nr;
    }
}
 
void reverse_transaction(LC L, char *card_nr_source, char *card_nr_dest, char *sum, FILE *f)
{

    char string[100];
    //cauta sursa si destinatia in baza de date
    LSC source, dest;
    for(; L != NULL; L = L->urm)
    {
        LSC aux = L->card_list;
        for(; aux != NULL; aux = aux->urm)
        {
            if(strcmp(aux->card_nr, card_nr_source) == 0)
                source = aux;
            if(strcmp(aux->card_nr, card_nr_dest) == 0)
                dest = aux;
        }
    }
    //verifica daca destinatia are destul bani pe card
    if(dest->balance < atoi(sum))
    {
        fprintf(f, "The transaction cannot be reversed\n");
    }
    else
    {
        source->balance += atoi(sum);
        dest->balance -= atoi(sum);
        char str[100];
        //copiez mesajul aferent transferului intr-un string
        sprintf(str, "SUCCESS, transfer_funds %s %s %s", source->card_nr, dest->card_nr, sum);
        LHistory ant, p = dest->history;
        //caunt acest string in lista de history
        for(ant = NULL; p != NULL; ant = p, p = p->urm)
        {
            if(strcmp(str, p->string) == 0)
            {
                break;
            }
        }
        //elimin celula respectiva
        if(p == NULL)
            return;
        if(ant == NULL)
            dest->history = p->urm;
        else
            ant->urm = p->urm;
        //adaug in istoricul cardului sursa mesajul de  mai jos
        sprintf(string, "SUCCESS, reverse_transaction %s %s %s", card_nr_source, card_nr_dest, sum);
        InsHistory(&source->history, string);
        freeHistory(&p);
    }
}
//functiad e mai jos intaorce suma de bani de pe cardul dorit,
//iar apoi adauga in istoricul cardului operatia
void balance_inquiry(LC L, char *card_nr, FILE *f)
{
    char string[100];
    for(; L != NULL; L = L->urm)
    {
        LSC aux = L->card_list;
        for(; aux != NULL; aux = aux->urm)
            if(strcmp(aux->card_nr, card_nr) == 0)
            {
                fprintf(f, "%d\n", aux->balance );
                sprintf(string, "SUCCESS, balance_inquiry %s", card_nr);
                InsHistory(&aux->history, string);
            }
    }
}
//functia de mai jos schimba pinul cardului dorit
void pin_change(LC L, char *card_nr, char *pin, FILE *f)
{
    char string[100]; int verif = 0;
    //verifica daca pinul contine litere
    if(pin[0] > 57 || pin[1] > 57 || pin[2] > 57 || pin[3] > 57)
        verif = 1;
    //cauta cardul in baza de date
    for(; L != NULL; L = L->urm)
    {
        LSC aux = L->card_list;
        for(; aux != NULL; aux = aux->urm)
            if(strcmp(aux->card_nr, card_nr) == 0)
            {
                if(strlen(pin) != 4 || verif == 1)
                {
                    fprintf(f, "Invalid PIN\n");
                    sprintf(string, "FAIL, pin_change %s %s", card_nr, pin);
                    InsHistory(&aux->history, string);
                }
                else
                {
                    sprintf(string, "SUCCESS, pin_change %s %s", card_nr, pin);
                    InsHistory(&aux->history, string);
                    strcpy(aux->status, "ACTIVE");
                    strcpy(aux->pin, pin);
                }
 
            }
    }  
}
//functia de mai jos adauga mesajul dorit in istoricul cardului
void cancel(LC L, char *card_nr)
{
    char string[100];
    for(; L != NULL; L = L->urm)
    {
        LSC aux = L->card_list;
        for(; aux != NULL; aux = aux->urm)
            if(strcmp(aux->card_nr, card_nr) == 0)
            {
                sprintf(string, "SUCCESS, cancel %s", card_nr);
                InsHistory(&aux->history, string);
            }
    }
}
//functia de mai jos deblocheaza cardul, modificand statusul acestuia
//si resezeteaza numarul de incercari de introducere al pinului
void unblock_card(LC L, char *card_nr)
{
    for(; L != NULL; L = L->urm)
    {
        LSC p = L->card_list;
        for(; p != NULL; p = p->urm)
        {
            if(strcmp(p->card_nr, card_nr) == 0)
            {
                memcpy(p->status, "ACTIVE", 6);
                p->nr_incercari_pin = 0;
            }
        }
    }
}
//functia de mai jos insereaza un card in bancomat
void insert_card(LC L, char *card_nr, char *pin, FILE *f)
{
    char string[100];
    //cauta cardul in baza de date
    for(; L != NULL; L = L->urm)
    {
        LSC p = L->card_list;
        for(; p != NULL; p = p->urm)
        {
            if(strcmp(p->card_nr, card_nr) == 0)
            {
            	//daca acesta este blocat se insereaza in istoric mesajul dorit
                if(strcmp(p->status, "LOCKED") == 0)
                {
                    fprintf(f, "The card is blocked. Please contact the administrator.\n");
                    sprintf(string, "FAIL, insert_card %s %s", p->card_nr, pin);
                    InsHistory(&p->history, string);
                    return;
                }
                //daca pinul cardului corespunde cu cel introdus, se adauga in istoric operatiunea
                if(strcmp(p->pin, pin) == 0)
                {
                    if(strcmp(p->status, "NEW") == 0)
                    {
                        fprintf(f, "You must change your PIN.\n");
                    }
                    sprintf(string, "SUCCESS, insert_card %s %s", p->card_nr, pin);
                    InsHistory(&p->history, string);
                    p->nr_incercari_pin = 0;
                   
                }
                //in caz contrar, retin in campul "nr_incercari_pin" de cate ori a fost introdus pinul gresit
                //la 3 incercari cardul se blocheaza si se reseteaza numarul de incercari
                else
                {
                    fprintf(f, "Invalid PIN\n");
                    sprintf(string, "FAIL, insert_card %s %s", p->card_nr, pin);
                    InsHistory(&p->history, string);
                    p->nr_incercari_pin++;
                    if(p->nr_incercari_pin == 3)
                    {
                        fprintf(f, "The card is blocked. Please contact the administrator.\n");
                        p->nr_incercari_pin = 0;
                        strcpy(p->status, "LOCKED");
                    }
                }
            }
        }
    }
}
//functia de mai jos redimensioneaza baza de date
void redimensionare(LC L, int nr_max_card)
{
    int  i;
    LSC r = NULL, aux;
    LC a = L;
    //in for-ul de mai jos, adaug cardurile din baza de data intr-o noua lista "r"
    for(i = 0; i < nr_max_card; i++)
    {
        for(L = a; L != NULL; L = L->urm)
        {
            LSC p = L->card_list, ant = NULL, u;
            while(p != NULL)
            {
                aux = p;
                p = p->urm;
                if(r == NULL)
                    r = aux;
                else
                    u->urm = aux;
                u = aux;
                if(ant == NULL)
                    L->card_list = p;
                else
                    ant->urm = p;
                if(r) u->urm = NULL;
            }
        }
 
    }
    //trec din nou la inceputul listei L
    L = a;
    //dupa aceea mut cardurile inapoi in baza de date pe pozitia noua
    for(; r != NULL; r = r->urm)
    {
        aux_add_card(&L, nr_max_card, r->card_nr, r->pin, r->expiry_date, r->CVV,
         r->history, r->status, r->balance, r->nr_incercari_pin);
        LSC q;
        q = r;
        free(q->history);
        free(q);
    }
 
}
 
int main()
{
    int nr_max_card;
    FILE *f = fopen("input.in","rt"), *f2 = fopen("output.out", "wt");
    if(!f) return 0;
    LC L;
    //citesc numarul maxim de carduri
    fscanf(f,"%d",&nr_max_card );
    fgetc(f); // scap de \n
    //citesc din fisier linie cu linie
    char buffer[1000], s[1000][1000]; int i = 0;
    while(fgets(buffer, 1000, f))
    {
        buffer[strlen(buffer) - 1] = '\0';
        strcpy(s[i],buffer);
        i++;
    }
 
    int j, ok = 0;
    int nr_carduri = 0;
    //parcurg liniile citite
    //pentru a extrage parametrii ceruti de pe linie utilizez strtok
    for(j = 0; j < i; j++)
    {
        char *operatie = strtok(s[j], " ");
        if(strcmp(operatie, "add_card") == 0)
        {
            if(ok == 0)
            {
                L = AlocareLC();
                ok = 1;
            }
            //extrag parametii ceruti de  pe linie
            char *card_nr = strtok(NULL, " "), *pin = strtok(NULL, " "),
            *expiry_date = strtok(NULL, " "), *CVV = strtok(NULL, " ");
            //daca cardul exista deja in baza de date afisez mesajul
            //de ficare data cand se adauga un nou card variabila nr_carduri creste
            if (exista_card (card_nr, L) == 0)
            {
                fprintf(f2, "The card already exists\n");
                continue;
            }
                nr_carduri++;
            //cand nr_carduri depaseste nr_max card, se dubleaza nr_max_card si se apeleaza
            //functia de redimensionare
            if(nr_carduri > nr_max_card)
            {
 
                nr_max_card = 2 * nr_max_card;
                redimensionare(L, nr_max_card);
            }
            //daca nu s-a reusit adaugarea unui card, scad variabila nr_carduri
            if(add_card(&L, nr_max_card, card_nr, pin, expiry_date, CVV, f2) == -1)
                nr_carduri--;
        }
        //mai jos apelez functiile cerute in functie de valoarea stringului "operatie"
        if(strcmp(operatie, "show") == 0)
        {
            char *card_nr = strtok(NULL, " ");
            if(!card_nr)
                show(L, f2);
            if(card_nr)
                show_card(L, card_nr, f2);
        }
        if(strcmp(operatie, "delete_card") == 0)
        {
        	//daca se sterge cardul respectiv se scade variabila nr_carduri cu 1
            char *card_nr = strtok(NULL, " ");
            delete_card(&L, card_nr);
            nr_carduri--;
        }
        if(strcmp(operatie, "insert_card") == 0)
        {
            char *card_nr = strtok(NULL, " ");
            char *pin = strtok(NULL, " ");
            insert_card(L, card_nr, pin, f2);
        }
        if(strcmp(operatie, "recharge") == 0)
        {
            char *card_nr = strtok(NULL, " ");
            char *sum = strtok(NULL, " ");
            recharge(L, card_nr, sum, f2);
        }
        if(strcmp(operatie, "cash_withdrawal") == 0)
        {
            char *card_nr = strtok(NULL, " ");
            char *sum = strtok(NULL, " ");
            cash_withdrawal(L, card_nr, sum, f2);
        }
        if(strcmp(operatie, "cancel") == 0)
        {
            char *card_nr = strtok(NULL, " ");
            cancel(L, card_nr);
        }
        if(strcmp(operatie, "pin_change") == 0)
        {
            char *card_nr = strtok(NULL, " ");
            char *pin = strtok(NULL, " ");
            pin_change(L, card_nr, pin, f2);
        }
        if(strcmp(operatie, "balance_inquiry") == 0)
        {
            char *card_nr = strtok(NULL, " ");
            balance_inquiry(L, card_nr, f2);
        }
        if(strcmp(operatie, "unblock_card") == 0)
        {
            char *card_nr = strtok(NULL, " ");
            unblock_card(L, card_nr);
        }
        if(strcmp(operatie, "transfer_funds") == 0)
        {
            char *source = strtok(NULL, " ");
            char *dest = strtok(NULL, " ");
            char *sum = strtok(NULL, " ");
            transfer_funds(L, source, dest, sum, f2);
        }
        if(strcmp(operatie, "reverse_transaction") == 0)
        {
            char *source = strtok(NULL, " ");
            char *dest = strtok(NULL, " ");
            char *sum = strtok(NULL, " ");
            reverse_transaction(L, source, dest, sum, f2);
        }
    }
 	//eliberez memoria
    freeLC(&L);
    fclose(f); fclose(f2);
    return 0;
}
