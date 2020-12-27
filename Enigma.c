//Luka Pavlovic NRT 18/19
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

//ne postoji bool u visual studio 2010 c-u jer nisu potpuno primili C99 standard
typedef int bool;
#define false 0
#define true 1
#define num_letters 26

//https://qph.fs.quoracdn.net/main-qimg-c81a5e53aedf0235117574f6469dc6fd
struct Enigma
{
	int rotor[3][num_letters];
	int reflector[num_letters];
	int plugboard[num_letters];
	int pos[3];
};

//enkodiranje?, input preko fajla?, output preko fajla?, fajl za konfiguraciju, fajl za input, fajl za output, index od argv za input preko teksta
struct Settings
{
	bool encode;
	bool inpf;
	bool outf;
	FILE *fconf;
	FILE *fin;
	FILE *fout;
	int index;
};

void initialize(struct Enigma *e, struct Settings *s);
void generate_conf();
void generate_array(int *array);
void eoferror();
void arguments(struct Enigma *e, struct Settings *s, int argc, char *argv[]);
void printmanual();
void printhelp();
void printusage();
FILE *open_file(char *name, char *mode);
void process_text(struct Enigma *e, struct Settings *s, char *word);
char encodechar(struct Enigma *e, int c);
int check(int x, int y);
void output_message(bool output_file);
void output_text(char c, struct Settings *s);


int main(int argc, char *argv[])
{
	struct Enigma *e;
	struct Settings *s;
	srand((unsigned int)time(0));

	if(argc == 1)
		printmanual();
	
	//dinamicko dodeljivanje jer je trazeno u zadatku
	e = (struct Enigma *) malloc(sizeof(struct Enigma));
	s = (struct Settings *) malloc(sizeof(struct Settings));
	
	initialize(e,s);	
	arguments(e,s,argc,argv);	
	process_text(e,s,argv[(*s).index]);

	free(e);
	free(s);
}

//inicijalizacija podataka i provera .conf fajla
void initialize(struct Enigma *e, struct Settings *s)
{
	int i, j;
	char check[25];
	
	printf("\nInicijalizacija podataka...");
	//provera da li enigma.conf fajl postoji
	(*s).fconf = fopen("enigma.conf","r");
	if((*s).fconf == NULL)
	{
		printf("\nIzgleda da nemate konfiguraciju.");
		generate_conf();
		(*s).fconf = open_file("enigma.conf","r");
		printf("\nGenerisanje konfiguracije (enigma.conf) uspesno zavrseno.");
	}
	
	//uzimamo podatke iz config fajla
	for(i = 0; i < 5; i++)
	{
		fscanf((*s).fconf,"%s ",check);
		if(check[0] == EOF || check[strlen(check)-1] != ':')
		{
			eoferror();
		}
		for(j = 0; j < num_letters; j++)
		{
			if (i == 0)
			{
				if(fscanf((*s).fconf,"%d ",&(*e).plugboard[j]) == EOF)
					eoferror();
			}
			if (i == 1)
			{
				if(fscanf((*s).fconf,"%d ",&(*e).reflector[j]) == EOF)
					eoferror();
			}
			if (i == 2)
			{
				if(fscanf((*s).fconf,"%d ",&(*e).rotor[i-2][j]) == EOF)
					eoferror();
			}
			if (i == 3)
			{
				if(fscanf((*s).fconf,"%d ",&(*e).rotor[i-2][j]) == EOF)
					eoferror();
			}
			if (i == 4)
			{
				if(fscanf((*s).fconf,"%d ",&(*e).rotor[i-2][j]) == EOF)
					eoferror();
			}
			
		}
	}

	(*e).pos[0] = 0;
	(*e).pos[1] = 0;
	(*e).pos[2] = 0;

	(*s).index = 0;
	(*s).encode = true;
	(*s).inpf = false;
	(*s).outf = false;
	
	fclose((*s).fconf);
}

/*
 * generisanje enigma.conf fajla
 * ceiling predstavlja velicinu pomocnog niza, takodje se koristi kako bismo redom prosli sve brojeve od 25 do 0
 * pomocni niz se koristi kako bismo pratili koji brojevi jos nisu iskoristeni
 * val1 i val2 su korisceni radi lakseg razumevanja algoritma
 */
void generate_conf()
{
	int i,j,ceiling = 25,ind1,ind2,val1,val2, plugboard[num_letters], reflector[num_letters], rotor[3][num_letters], pomniz[num_letters] = {0};
	FILE *fconf;
	fconf = open_file("enigma.conf","w");
	
	/*
	 * generate plugboard
	 * broj indeksa predstavlja slovo (0-25), vrednost indeksa je indeks (slovo) koji je rezultat
	 * nalazimo dve nasumicne vrednosti i povezujemo ih (a[25] = 1 && a[1] = 25)
	 * ovo se radi deset puta jer je u praksi bilo deset kablova za povezivanje slova
	 */
	generate_array(pomniz);
	while(ceiling > 5)
	{
		ind1 = rand() % (ceiling+1);
		ind2 = rand() % (ceiling+1);
		if(ind1 != ind2)
		{
		    val1 = pomniz[ind1];
		    val2 = pomniz[ind2];
			plugboard[val1] = val2;
			plugboard[val2] = val1;
			
			//brisanje prve vrednosti
			for(i=ind1; i<ceiling; i++)
			{
				pomniz[i] = pomniz[i + 1];
			}
			ceiling--;
			//brisanje druge vrednosti
			if(ind1 < ind2)
			{
			    for(i=ind2-1; i<ceiling; i++)
			    {
				    pomniz[i] = pomniz[i + 1];
			    }
			}
			else
			{
			    for(i=ind2; i<ceiling; i++)
			    {
				    pomniz[i] = pomniz[i + 1];
			    }
			}
			
			ceiling--;

		}
	}
	//preostalih 6 vrednosti imaju vrednost svog indeksa (a[25] = 25, odnosno Z = Z)
	for(i = 0; i < ceiling + 1; i++)
	{
	    plugboard[pomniz[i]] = pomniz[i];
	}
	//upisivanje niza u .conf fajl
	fprintf(fconf,"plugboard: ");
	for(i = 0; i < num_letters; i++)
	{
	    fprintf(fconf,"%d ",plugboard[i]);
	}


	/*
	 * generate reflector
	 * broj indeksa predstavlja slovo (0-25), vrednost indeksa je indeks (slovo) koji je rezultat
	 * nalazimo dve nasumicne vrednosti i povezujemo ih (a[25] = 1 && a[1] = 25)
	 */
	ceiling = 25;
	generate_array(pomniz);
	while(ceiling > 0)
	{
		ind1 = rand() % (ceiling+1);
		ind2 = rand() % (ceiling+1);
		if(ind1 != ind2)
		{
		    val1 = pomniz[ind1];
		    val2 = pomniz[ind2];
			reflector[val1] = val2;
			reflector[val2] = val1;
			
			for(i=ind1; i<ceiling; i++)
			{
				pomniz[i] = pomniz[i + 1];
			}
			ceiling--;
			if(ind1 < ind2)
			{
			    for(i=ind2-1; i<ceiling; i++)
			    {
				    pomniz[i] = pomniz[i + 1];
			    }
			}
			else
			{
			    for(i=ind2; i<ceiling; i++)
			    {
				    pomniz[i] = pomniz[i + 1];
			    }
			}
			
			ceiling--;

		}
	}
	//upisivanje niza u .conf fajl
	fprintf(fconf,"\nreflector: ");
	for(i = 0; i < num_letters; i++)
	{
	    fprintf(fconf,"%d ",reflector[i]);
	}

	
	/*
	 * generate rotors
	 * indeks niza predstavlja slovo (0-25), dok vrednost niza predstavlja koliko treba da se skoci od indeksa da bi se doslo do rezultata
	 * algoritam je znatno drugaciji jer se rotor nakon svakog slova rotira za jedno mesto
	 */
	for(i = 0; i < 3; i++)
    {   
        ceiling = 25;
        generate_array(pomniz);
        while(ceiling >= 0)
        {
			//provera kako poslednji indeks (0) ne bi ostao bez para
			if(ceiling == 1 && pomniz[0] == 0)
			{
				rotor[i][0] = pomniz[1];
				rotor[i][1] = 25;
				break;
			}
            ind1 = rand() % (ceiling+1);
            if(pomniz[ind1] != ceiling)
		    {
		        if(ceiling > pomniz[ind1])
                    rotor[i][ceiling] = pomniz[ind1] + (num_letters - ceiling);
		        else
		            rotor[i][ceiling] = pomniz[ind1] - ceiling;
				//brisanje broja koji je postavljen u rotor
		        for(j=ind1; j<ceiling; j++)
			    {
				    pomniz[j] = pomniz[j + 1];
			    }
		        ceiling--;
		    }
        }
    }
	//upisivanje nizova u .conf fajl
	fprintf(fconf,"\nrotor1: ");
	for(i = 0; i < num_letters; i++)
	{
	    fprintf(fconf,"%d ",rotor[0][i]);
	}
	fprintf(fconf,"\nrotor2: ");
	for(i = 0; i < num_letters; i++)
	{
	    fprintf(fconf,"%d ",rotor[1][i]);
	}
	fprintf(fconf,"\nrotor3: ");
	for(i = 0; i < num_letters; i++)
	{
	    fprintf(fconf,"%d ",rotor[2][i]);
	}

	fclose(fconf);
}

//inicijalizacija (pomocnog) niza
void generate_array(int *array)
{
	int i;
	for(i=0; i < num_letters; i++)
	{
		array[i] = i;
	}
}

//greska ukoliko je nadjen EOF pre nego sto bi trebalo
void eoferror()
{
	printf("\nGreska u otvaranju .conf fajla, probajte -g kako bi se opet izgenerisao\n");
	exit(1);
}

//rasclanjuje argumente, postoje neke verzije getopt.h za windows ali bi to komplikovalo predaju zadatka jer sve mora biti u jednom .c fajlu
void arguments(struct Enigma *e, struct Settings *s, int argc, char *argv[])
{
	int i,j = 0;
	printf("\nRazlaganje argumenata...");
	
	for(i = 1; i < argc; i++)
	{
		//help
		if(strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0)
		{
			printhelp();
		}
		//manual
		if(strcmp(argv[i],"-m") == 0 || strcmp(argv[i],"--manual") == 0)
		{
			printmanual();
		}
		//manual
		if(strcmp(argv[i],"-u") == 0 || strcmp(argv[i],"--usage") == 0)
		{
			printusage();
		}
		//generate (conf)
		if(strcmp(argv[i],"-g") == 0 || strcmp(argv[i],"--generate") == 0)
		{
			generate_conf();
			printf("\nGenerisanje konfiguracije (enigma.conf) uspesno zavrseno.");
			exit(1);
		}
		//encode
		if(strcmp(argv[i],"-e") == 0 || strcmp(argv[i],"--encode") == 0)
		{
			(*s).encode = true;
		}
		//decode
		else if(strcmp(argv[i],"-d") == 0 || strcmp(argv[i],"--decode") == 0)
		{
			(*s).encode = false;
		}
		//input text
		else if(strcmp(argv[i],"-it") == 0 || strcmp(argv[i],"--inputtext") == 0)
		{
			i++;
			(*s).index = i;
		}
		//output text
		else if(strcmp(argv[i],"-ot") == 0 || strcmp(argv[i],"--outputtext") == 0)
		{
			(*s).outf = false;
		}
		//input file name
		else if(strcmp(argv[i],"-if") == 0 || strcmp(argv[i],"--inputfile") == 0)
		{
			(*s).inpf = true;
			i++;
			(*s).fin = open_file(argv[i],"r");
		}
		//output file name
		else if(strcmp(argv[i],"-of") == 0 || strcmp(argv[i],"--outputfile") == 0)
		{
			(*s).outf = true;
			i++;
			(*s).fout = open_file(argv[i],"w");
		}
		//code
		else if(strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"--code") == 0)
		{
			j = i;
			while(j+3 > i)
			{
				i++;
				(*e).pos[i - j - 1] = atoi(argv[i]);
				if((*e).pos[i - j - 1] < 0 || (*e).pos[i - j - 1] > num_letters - 1)
				{
					printf("\nSvaki deo koda mora od 0 do 25!\n");
					exit(1);
				}
			}
		}
		//uzima index od input teksta
		else
		{
			(*s).index = i;
		}

	}
}

//pocetni tekst
void printmanual()
{
	printf("\n\nDobro dosli, ovaj program se iskljucivo moze koristiti u komandnoj liniji (CMD). \n"
			"Enigma je program baziran na linux alatima(npr. base64) koji generise Vasu jedinstvenu enigma masinu, kodira/dekodira unos i ispisuje vrednost u zeljenom formatu.\n"
			"Program daje korisniku veliku fleksibilnost, od brze enkripcije reci do potpuno izmenljive konfiguracije koja daje istu kompleksnost kao i prava masina (10^23).\n"
			"Preporuka je da prvo pogledate primere koriscenja programa sa argumentom -u (--usage)"
			"\nAko zelite da vidite sve argumente i sta oni rade, iskoristite -h (--help)\n"
			"Ukoliko zelite da se ovaj tekst prikaze opet, iskoristite -m (--manual)\n");
	exit(1);
}

//pomoc
void printhelp()
{
	printf("\n-h, --help: Prikazuje tekst za pomoc i izlazi iz programa\n"
			"-u, --usage: Prikazuje tekst sa primerima i izlazi iz programa\n"
			"-m, --manual: Prikazuje pocetni tekst i izlazi iz programa\n"
			"-g, --generate: Generise novu enigma konfiguraciju i izlazi iz programa\n"
			"-e, --encode: (Podrazumevano) Tekst treba da se enkriptuje\n"
			"-d, --decode: Tekst treba da se dekriptuje\n"
			"-it <\"Tekst\">, --inputtext: (Podrazumevano)Ulazni tekst preko tastature\n"
			"-if <fajl.txt>, --inputfile: Ulazni tekst preko fajla\n"
			"-ot, --outputtext: (Podrazumevano)Izlazni tekst ce biti prikazan na ekranu\n"
			"-of <fajl.txt>, --outputfile: Izlazni tekst ce biti sacuvan u fajl\n"
			"-c <broj broj broj>, --code: Kod koji ce biti koriscen za enkripciju\n\n");
	exit(1);
}

//nacin koriscenja
void printusage(){
	printf("\n\nPrimer 1.1: Enigma.exe -e \"Danas je vedro\"\n"
			"Enkriptuje se tekst \"Danas je vedro\" i kodiran tekst izlazi na ekran.\n"
			"Primer 1.2: Enigma.exe -d \"Clicb px bcldm\"\n"
			"Dekriptuje se tekst koji smo prethodno dobili na ekranu.\n\n"
			"Primer 2.1: Enigma.exe -e -it \"Danas je vedro\" -of primer.txt\n"
			"Kodira se tekst koji smo uneli (--inputtext) i rezultat se ispisuje u primer.txt (--outputfile)\n"
			"Primer 2.2: Enigma.exe -d -if primer.txt -ot\n"
			"Uzimamo (enkriptovanu) vrednost iz primer.txt (--inputfile) i ispisujemo je na ekran (--outputtext)\n\n"
			"Primer 3.1 Enigma.exe -e \"Primer koriscenja koda\" -c 21 0 15\n"
			"Kodiramo tekst (-it i -ot su podrazumevani) sa kodom 21 0 15 (kod se uvek sastoji od tri broja od 0 do 26)\n"
			"Primer 3.2 Enigma.exe -d \"Tyzvlh jyhmqtoenc euse\" -c 21 0 15\n"
			"Dekodiramo tekst sa istim kodom kako bi dobili istu vrednost kao i prvobitni ulaz. \n"
			"Koriscenjem -c (ili --code) drasticno povecavamo nivo kompleksnosti. "
			"Iako nece biti objasnjen tacan nacin u ovom tekstu, koriscenjem -c i rucnim menjanjem konfiguracionog fajla mozemo simulirati identicne konfiguracije koje su imali nacisti tokom drugog svetskog rata. \n");
	exit(1);
}

//otvaranje fajlova
FILE *open_file(char *name, char *mode){
	FILE *f = fopen(name, mode);

	if(f == NULL){
		printf("\nDoslo je do greske prilikom otvaranja datoteke!\n");
		exit(1);
	}

	return f;
}

//enkripcija/dekripcija i ispis
void process_text(struct Enigma *e, struct Settings *s, char *word)
{
	int c = 0, i = 0;
	
	printf("\nProcesiranje teksta...");
	printf("\nVas kod je %d %d %d, nemojte ga zaboraviti!",(*e).pos[0],(*e).pos[1],(*e).pos[2]);
	output_message((*s).outf);
	
	if((*s).inpf == true)
	{
		while ((c = getc((*s).fin)) != EOF)
		{
			c = encodechar(e,c);
			output_text(c,s);
		}
		fclose((*s).fout);
	}
	else
	{
		c = word[0];
		while (c != '\0')
		{
			c = encodechar(e,c);
			output_text(c,s);
			i++;
			c = word[i];			
		}
	}

	if((*s).outf == true)
		fclose((*s).fout);
	printf("\n");
}

//enkriptovanje teksta, -e i -d zapravo ne rade nista vec su tu da olaksaju logiku korisniku jer je proces enkriptovanja i dekriptovanja isti, pogledati sliku pri vrhu za primer
char encodechar(struct Enigma *e, int c)
{
	int i=0,j,k;
	bool upper;
	upper = false;
	if(isalpha(c) != 0)
	{
		if(isupper(c) == true)
			upper = true;
		//unos -> plugboard -> rotor1 -> rotor2 -> rotor3 -> reflector -> (od nazad) rotor3 -> (od nazad) rotor2 -> (od nazad) rotor1 -> (od nazad) plugboard -> izlaz
		c = toupper(c) - 65;
		c = (*e).plugboard[c];
		c = check(c,(*e).rotor[0][check(c,(*e).pos[0])]);
		c = check(c,(*e).rotor[1][check(c,(*e).pos[1])]);
		c = check(c,(*e).rotor[2][check(c,(*e).pos[2])]);
		c = (*e).reflector[c];
		for(j = 2; j >= 0; j--)
		{
			for(k = 0; k < num_letters; k++)
			{
				if(check(k,(*e).rotor[j][check(k,(*e).pos[j])]) == c)
				{
					c = k;
					k = num_letters;
				}
			}
		}
		for(k = 0; k < num_letters; k++)
		{
			if((*e).plugboard[k] == c)
			{
				c = k;
				k = num_letters;
			}
		}
		
		//rotacija rotora
		(*e).pos[0]++;
		if((*e).pos[0] > num_letters-1)
		{
			(*e).pos[0] = 0;
			(*e).pos[1]++;
		}
		if((*e).pos[1] > num_letters-1)
		{
			(*e).pos[1] = 0;
			(*e).pos[2]++;
		}
		if((*e).pos[2] > num_letters-1)
		{
			(*e).pos[0] = 0;
			(*e).pos[1] = 0;
			(*e).pos[2] = 0;
		}
		if(upper == true)
			c = c + 65;
		else
			c = c + 97;
	}
	return c;
}

//provera da li je vrednost iznad 25 (ako jeste smanjujemo)
int check(int x, int y)
{
	if(x + y > num_letters - 1)
	{
		return (x+y-num_letters);
	}
	else return x+y;
}

//poruka za rezultat
void output_message(bool output_file)
{
	if(output_file == true)
		printf("\nRezultat masine je postavljen u fajl.");
	else
		printf("\nRezultat masine je: \n");
}

//izlaz enkriptovanog teksta u fajl ili na ekran
void output_text(char c, struct Settings *s)
{
	if((*s).outf == true)
		fprintf((*s).fout,"%c",c);
	else
		printf("%c",c);
}