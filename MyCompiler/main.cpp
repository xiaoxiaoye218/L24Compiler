/*
 * PL/0 complier program implemented in C
 * PL/0�������������汾��
 * The program has been tested on Visual Studio 2022
 *
 * ʹ�÷�����
 * ���к�����PL/0Դ�����ļ���
 * �ش��Ƿ�������������
 * �ش��Ƿ�������ű�
 * fcode.txt������������
 * foutput.txt���Դ�ļ�������ʾ�⣨���д��͸��ж�Ӧ�����ɴ����׵�ַ�����޴�
 * fresult.txt������н��
 * ftable.txt������ű�
 */
#define  _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>


#define bool int
#define true 1
#define false 0

#define norw 10       /* �����ָ��� */
#define txmax 100     /* ���ű����� */
#define nmax 14       /* ���ֵ����λ�� */
#define al 10         /* ��ʶ������󳤶� */
#define maxerr 30     /* ������������� */
#define amax 2048     /* ��ַ�Ͻ�*/
#define levmax 3      /* ����������Ƕ����������*/
#define cxmax 200     /* ��������������� */
#define stacksize 500 /* ����ʱ����ջԪ�����Ϊ500�� */

 /* ���� */
//�����֣�13 ������ʶ����ident�����֣�number��������ͷָ��������� plus, minus, times, slash, eql, neq, lss, leq, gtr, geq, lparen, rparen, comma, semicolon, period, becomes��������ţ�nul��
enum symbol {
	nul, ident, number,	conststr,			//��ʶ�������֡�""�ַ���	var--number, strsym--conststr
	plus, minus, times, slash,				//+��-��*��/
	becomes, eql, neq,lss, leq, gtr, geq,	//=, ==, !=, <, <=, >, >=
	lparen, rparen, lbrace, rbrace,			//(, ), {, }
	comma, semicolon, 						//, ; 
	ifsym, thensym, elsesym, endsym,		//if, then, else, endd
	whilesym,								//while
	printsym, scansym,						//ԭ����writesym, readsym, 
	varsym, strsym,							//����constsym��procsym������strsym
	mainsym, 
};
#define symnum 31

/* ���ű��е����� */
enum object 
{
	variable,
	stringvar,
};

/* ���������ָ�� */
enum fct {
	lit, opr, lod,
	sto, cal, ini,
	jmp, jpc,
	lits, lods, stos
};
#define fctnum 8			//ָ��ĸ���

/* ���������ṹ */
struct instruction
{
	enum fct f; /* ���������ָ�� */
	int l;      /* ���ò���������Ĳ�β� */
	int a;      /* ����f�Ĳ�ͬ����ͬ */
	std::string s;
};


bool listswitch;   /* ��ʾ������������ */
bool tableswitch;  /* ��ʾ���ű���� */
char ch;            /* ��ŵ�ǰ��ȡ���ַ���getch ʹ�� */
enum symbol sym;    /* ��ǰ�ķ��� */
char id[al + 1];      /* ��ǰident�������һ���ֽ����ڴ��0 */
int num;            /* ��ǰnumber */
std::string str;	
int cc, ll;         /* getchʹ�õļ�������cc��ʾ��ǰ�ַ�(ch)��λ�� */
int cx;             /* ���������ָ��, ȡֵ��Χ[0, cxmax-1]*/
char line[81];      /* ��ȡ�л����� */
char a[al + 1];       /* ��ʱ���ţ������һ���ֽ����ڴ��0 */
struct instruction code[cxmax]; /* ����������������� */
char word[norw][al];        /* ������ */					//[�����ָ���][��������󳤶�]����init�У��ǰ��ֵ�������洢�ģ�������ֲ���
enum symbol wsym[norw];     /* �����ֶ�Ӧ�ķ���ֵ */		//������������word�е����� �� enum symbol�е�ֵ�Ķ�Ӧ��ϵ
enum symbol ssym[256];      /* ���ַ��ķ���ֵ */
char mnemonic[fctnum][5];   /* ���������ָ������ */
bool declbegsys[symnum];    /* ��ʾ������ʼ�ķ��ż��� */	//����������������������������
bool statbegsys[symnum];    /* ��ʾ��俪ʼ�ķ��ż��� */	//��ֵ��䡢������䡢ѭ�����
bool facbegsys[symnum];     /* ��ʾ���ӿ�ʼ�ķ��ż��� */	//��ʶ�������֡����ű��ʽ
bool strfacbegsys[symnum];

/* ���ű�ṹ */
struct tablestruct
{
	char name[al];	    /* ���� */
	enum object kind;	/* ���ͣ�constant��variable��procedure */
	int val;            /* ��ֵ����constʹ�� */
	int level;          /* �����㣬��const��ʹ�� */
	int adr;            /* ��ַ����const��ʹ�� */
	int size;           /* ��Ҫ������������ռ�, ��procedureʹ�� */
};

struct tablestruct table[txmax]; /* ���ű� */

FILE* fin;      /* ����Դ�ļ� */
FILE* ftable;	/* ������ű� */
FILE* fcode;    /* ������������ */
FILE* foutput;  /* ����ļ�������ʾ�⣨���д������ж�Ӧ�����ɴ����׵�ַ�����޴� */
FILE* fresult;  /* ���ִ�н�� */
char fname[al];
int err;        /* ��������� */

void error(int n);		//��ӡ������Ϣ����������������n��������
void getsym();			//symbol�����дʷ�����������ǰ��ȡ���ַ�ת��Ϊ����
void getch();			//��ȡ��һ���ַ����������кͿհ׷�
void init();	//��ʼ�������񡢼��Ϻͷ���
void gen(enum fct x, int y, int z,std::string="");			//�������������
void test(bool* s1, bool* s2, int n);		//���Ե�ǰ�����Ƿ�Ϸ�
int inset(int e, bool* s);		//���Ԫ���Ƿ��ڼ�����
int addset(bool* sr, bool* s1, bool* s2, int n);	//�����������ϵĲ���
int subset(bool* sr, bool* s1, bool* s2, int n);	//�
int mulset(bool* sr, bool* s1, bool* s2, int n);	//����
void block(int lev, int tx, bool* fsys);	//����ֳ���
void interpret();									//����ִ�����������
void factor(bool* fsys, int* ptx, int lev);			//��������
void term(bool* fsys, int* ptx, int lev);			//������
void condition(bool* fsys, int* ptx, int lev);		//��������
void expression(bool* fsys, int* ptx, int lev);		//������ʽ
void statement(bool* fsys, int* ptx, int lev);		//�������
void listcode(int cx0);									//���Ŀ�����
void listall();											//�������Ŀ�����
void vardeclaration(int* ptx, int lev, int* pdx);		//�����������
void constdeclaration(int* ptx, int lev, int* pdx);		//����������
int position(char* idt, int tx);				//���ұ�ʶ���ڷ��ű��е�λ��
void enter(enum object k, int* ptx, int lev, int* pdx);		//�ڷ��ű��м���һ��
int base(int l, int* s, int b);							//ͨ�����̻�ַ����һ����̵Ļ�ַ
void myerr(const char*);
void statementlist(bool* fsys, int* ptx, int lev);
void strdeclaration(int* ptx, int lev, int* pdx);
void strfactor(bool* fsys, int* ptx, int lev);			
void strterm(bool* fsys, int* ptx, int lev);			
void strcondition(bool* fsys, int* ptx, int lev);		
void strexpr(bool* fsys, int* ptx, int lev);		


#include <sstream>

// ������ת��Ϊ�ַ����ĸ�������
std::string intToString(int num) {
	std::stringstream ss;
	ss << num;
	return ss.str();
}

std::string scopy(std::string s, int n)
{
	std::string res;
	for (int i = 0; i < n; i++)
		res += s;
	return res;
}

/* ������ʼ */
int main()
{
	bool nxtlev[symnum];

	printf("Input pl/0 file?   ");
	scanf("%s", fname);		/* �����ļ��� */

	if ((fin = fopen(fname, "r")) == NULL)
	{
		printf("Can't open the input file!\n");
		exit(1);
	}

	ch = fgetc(fin);
	if (ch == EOF)
	{
		printf("The input file is empty!\n");
		fclose(fin);
		exit(1);
	}
	rewind(fin);		//�ز�ָ�뵽�ļ���ͷ

	if ((foutput = fopen("foutput.txt", "w")) == NULL)
	{
		printf("Can't open the output file!\n");
		exit(1);
	}

	if ((ftable = fopen("ftable.txt", "w")) == NULL)
	{
		printf("Can't open ftable.txt file!\n");
		exit(1);
	}

	printf("List object codes?(Y/N)");	/* �Ƿ������������� */
	scanf("%s", fname);
	listswitch = (fname[0] == 'y' || fname[0] == 'Y');

	printf("List symbol table?(Y/N)");	/* �Ƿ�������ű� */
	scanf("%s", fname);
	tableswitch = (fname[0] == 'y' || fname[0] == 'Y');


	//--------------------------------�ֽ���-----------------------------------


	init();		/* ��ʼ�� */
	err = 0;	//��¼��������з����Ĵ�������
	cc = ll = cx = 0;	//cc���ַ���������ll���г��ȣ�cx������ָ��
	ch = ' ';

	getsym();		//���ôʷ�������������Դ�ļ��л�ȡ��һ������token


	if (sym == mainsym)	//����һ��main{}ʶ���߼�
	{
		getsym();
		if (sym == lbrace)	//ȷ��{
		{
			//����main{����߼���ԭ��PL0��ͬ
			//��������ʼ���ź���俪ʼ���źϲ���nxtlev��
			//��Ϊ�ڽ��������У���Ҫʶ����Կ�ʼ���������ķ��ţ�����block����ʶ��ʹ���������������ʼλ��
			getsym();
			bool nxtlev[symnum];
			addset(nxtlev, declbegsys, statbegsys, symnum);
			nxtlev[rbrace] = true;

			block(0, 0, nxtlev);  /* ����ֳ��� */
		}
		else
			myerr("ȱ��'{'");
	}
	else 
	{
		myerr("ȱ��'main'");
	}



	if (err == 0)		//���û���κδ���
	{
		printf("\n===Parsing success!===\n");
		fprintf(foutput, "\n===Parsing success!===\n");

		if ((fcode = fopen("fcode.txt", "w")) == NULL)
		{
			printf("Can't open fcode.txt file!\n");
			exit(1);
		}

		if ((fresult = fopen("fresult.txt", "w")) == NULL)
		{
			printf("Can't open fresult.txt file!\n");
			exit(1);
		}

		listall();	 /* ������д��� */
		fclose(fcode);

		interpret();	/* ���ý���ִ�г��� */
		fclose(fresult);
	}
	else
	{
		printf("\n%d errors in pl/0 program!\n", err);
		fprintf(foutput, "\n%d errors in pl/0 program!\n", err);
	}

	fclose(ftable);
	fclose(foutput);
	fclose(fin);

	return 0;
}

/*
 * ��ʼ��
 */
void init()
{
	int i;

	/* ���õ��ַ����� */
	for (i = 0; i <= 255; i++)
	{
		ssym[i] = nul;
	}
	ssym['+'] = plus;		//����һ�����ַ��Ĺ�ϣӳ��	ssym[ch]=ch�ķ���ֵ
	ssym['-'] = minus;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['{'] = lbrace;
	ssym['}'] = rbrace;
	//ssym['='] = becomes;		//=��==,�����Ǻ�<һ�����߼���ͬ��!=����PL0���ǵ��ַ�#��������L24���������ַ�!=��Ҫ������<=���߼�; =�Ǹ�ֵ��==��eql
	ssym[','] = comma;
	ssym[';'] = semicolon;

	/* ���ñ���������,������ĸ˳�򣬱��ڶ��ֲ��� */
	//word[�����ָ���][��������󳤶�]
	//��������ַ��Ǻ�EBNF�����һ�µģ�����PL0��procedure����Ϊ�﷨��������
	//��������scan�Ͳ������Ըĳ�scanf��Ȼ���ַ������͵���������str xxx-------------
	strcpy(&(word[0][0]), "else");
	strcpy(&(word[1][0]), "end");
	strcpy(&(word[2][0]), "if");
	strcpy(&(word[3][0]), "main");
	strcpy(&(word[4][0]), "print");
	strcpy(&(word[5][0]), "scan");
	strcpy(&(word[6][0]), "str");
	strcpy(&(word[7][0]), "then");
	strcpy(&(word[8][0]), "var");
	strcpy(&(word[9][0]), "while");

	/* ���ñ����ַ��� */
	wsym[0] = elsesym;			//word���е�0���ַ���Ӧ��enum symbol����elsesym
	wsym[1] = endsym;
	wsym[2] = ifsym;
	wsym[3] = mainsym;
	wsym[4] = printsym;
	wsym[5] = scansym;
	wsym[6] = strsym;
	wsym[7] = thensym;
	wsym[8] = varsym;
	wsym[9] = whilesym;


	/* ����ָ������ */
	//mnemonic��ָ������Ƿ�����ǰ�涨���enum fct��mnemonic���ַ�����Ӧ����
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[ini][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");
	strcpy(&(mnemonic[lits][0]), "lits");

	/* ���÷��ż� */
	for (i = 0; i < symnum; i++)
	{
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/* ����������ʼ���ż� */
	declbegsys[varsym] = true;
	declbegsys[strsym] = true;
	//varsym������Ϊ�����Ŀ�ʼ����

	/* ������俪ʼ���ż� */
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[scansym] = true;
	statbegsys[printsym] = true;

	//beginsym,callsym,ifsym,whilesym�ֱ��ʾbegin��䡢call��䣬if��䣬while���

	/* �������ӿ�ʼ���ż� */
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;

	strfacbegsys[ident] = true;
	strfacbegsys[number] = true;
	strfacbegsys[conststr] = true;
	strfacbegsys[lparen] = true;
	//ident,number,lparen ��ʶ�������֡�������

	//��;��if��ǰ��������������ʼ���ż�����������Ӧ�����������߼���������������������������������
	//		if��ǰ����������俪ʼ���ż�����������Ӧ���������߼� 
}

/*
 * ������ʵ�ּ��ϵļ�������
 */
int inset(int e, bool* s)
{
	return s[e];
}

//�ϲ�������ʼ���ż�������洢��Ŀ�꼯��sr�У�n����ָ�����ϵĴ�С�������ŵ�����
int addset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		sr[i] = s1[i] || s2[i];
	}
	return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		sr[i] = s1[i] && (!s2[i]);
	}
	return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		sr[i] = s1[i] && s2[i];
	}
	return 0;
}

/*
 *	��������ӡ����λ�úʹ������
 */
void error(int n)
{
	char space[81];
	memset(space, 32, 81);

	space[cc - 1] = 0; /* ����ʱ��ǰ�����Ѿ����꣬����cc-1 */

	printf("**%s^%d\n", space, n);
	fprintf(foutput, "**%s^%d\n", space, n);

	err = err + 1;
	if (err > maxerr)
	{
		exit(1);
	}
}

void myerr(const char* s)
{
	char space[81];
	memset(space, 32, 81);

	space[cc - 1] = 0; /* ����ʱ��ǰ�����Ѿ����꣬����cc-1 */

	printf("**%s^%s\n", space, s);
	fprintf(foutput, "**%s^%s\n", space, s);

	err = err + 1;
	if (err > maxerr)
	{
		exit(1);
	}
}

/*
 * ���˿ո񣬶�ȡһ���ַ�
 * ÿ�ζ�һ�У�����line��������line��getsymȡ�պ��ٶ�һ��
 * ������getsym����
 */
void getch()
{
	//cc:current character position��ǰ�ַ���λ�ü���������ʾ��ǰ���ڴ�����ַ��ڻ������е��±�λ��
	//ll:line length ��ǰ�еĳ��ȼ�����
	//if(cc == ll) ������������ڵ��ַ������������ˣ����߶�Ϊ0Ҳ���ǻ�û����
	if (cc == ll) /* �жϻ��������Ƿ����ַ��������ַ����������һ���ַ����������� */
	{
		//feof()����ļ�ָ���Ƿ񵽴��ļ�ĩβ
		if (feof(fin))
		{
			printf("Program is incomplete!\n");
			exit(1);
		}
		ll = 0;
		cc = 0;
		//cx��code index����main�б���ʼ��Ϊ0��ָ��ǰָ���λ��
		printf("%d ", cx);
		fprintf(foutput, "%d ", cx);
		ch = ' ';
		//���ж��Ʊ������Ϊgetch��Ŀ���Ƕ�ȡ�����ַ���ֱ���������з��������հ��ַ�����Ӱ���еĽ����ж� 
		while (ch != 10)
		{
			//��fin��ȡһ���ַ����洢��ch�У������ȡ���ļ�ĩβ��fscanf����EOF��
			if (EOF == fscanf(fin, "%c", &ch))
			{
				line[ll] = 0;
				break;
			}

			printf("%c", ch);
			fprintf(foutput, "%c", ch);
			line[ll] = ch;
			ll++;				//��¼��һ�е��ַ�����
		}
	}
	//�������Ǵӻ�����line�л�ȡ��ǰ�ַ���ccָ����ǵ�ǰҪ������ַ��±�
	ch = line[cc];
	cc++;
}

/*
 * �ʷ���������ȡһ�����ţ����������ʹ洢��ȫ�ֱ���sym��
 */
void getsym()
{
	int i, j, k;
	///* ch��ŵ�ǰ��ȡ���ַ���getch ʹ�� */
	while (ch == ' ' || ch == 10 || ch == 9)	/* ���˿ո񡢻��к��Ʊ�� */
	{
		getch();
	}
	//��������string����Щ�����Գ�Ϊ��ʶ��
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) /* ��ǰ�ĵ����Ǳ�ʶ�����Ǳ����� */
	{
		k = 0;		//��ʱ������
		//a[]��ȫ�ֱ��������ڴ����ʱ����
		do {
			if (k < al)
			{
				a[k] = ch;
				k++;
			}
			getch();
		} while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'));
		a[k] = 0;		//������ĩβ�����ֹ��
		strcpy(id, a);	//����ʱ�ַ�����a[]���Ƶ���ʶ������id[]
		i = 0;
		j = norw - 1;		//norw�Ǳ�����word[]��������i��j��ʵ���Ƕ��ֲ��ҵ����ұ߽�
		do 
		{    /* ������ǰ�����Ƿ�Ϊ�����֣�ʹ�ö��ַ����� */
			k = (i + j) / 2;
			//strcmp�Ƚ������ַ�����������һ��������<=0��˵��id<=work[k]
			if (strcmp(id, word[k]) <= 0)
			{
				j = k - 1;
			}
			if (strcmp(id, word[k]) >= 0)
			{
				i = k + 1;
			}
		} while (i <= j);
		if (i - 1 > j) /* ��ǰ�ĵ����Ǳ����� */
		{
			sym = wsym[k];		//k���ǵڼ��������֣�wsym[k]��¼�˵�k�������ֵ�enumֵ
		}
		else /* ��ǰ�ĵ����Ǳ�ʶ�� */
		{
			sym = ident;		//ident�������enum symbol���һ��ֵ
		}
	}
	//ʶ���ַ���
	else if (ch == '"')
	{
		//�����ټ�string���ͳ����Ĵ�������number������var��var����������
		str = "";		//����str
		sym = conststr;
		getch();
		while (ch != '"' && ch != EOF)
		{
			str.push_back(ch);
			getch();
		}
		if (ch == '"')
			getch();
		else
			myerr("ȱ��\"");
	}
	else
	{
		if (ch >= '0' && ch <= '9') /* ��ǰ�ĵ��������� */
		{
			k = 0;
			num = 0;
			sym = number;
			do {
				num = 10 * num + ch - '0';
				k++;
				getch();;
			} while (ch >= '0' && ch <= '9'); /* ��ȡ���ֵ�ֵ */
			k--;
			if (k > nmax) /* ����λ��̫�� */
			{
				error(30);
			}
		}
		else
		{
			if (ch == '=')
			{
				getch();
				if (ch == '=')
				{
					sym = eql;
					getch();
				}
				else
				{
					sym = becomes;
				}
			}
			else if (ch == '!')
			{
				getch();
				if (ch == '=')
				{
					sym = neq;
					getch();
				}
				else
					sym = nul;		//����ʶ��ķ���
			}
			else if (ch == '<')		/* ���С�ڻ�С�ڵ��ڷ��� */
			{
				getch();
				if (ch == '=')
				{
					sym = leq;
					getch();
				}
				else
				{
					sym = lss;
				}
			}
			else if (ch == '>')		/* �����ڻ���ڵ��ڷ��� */
			{
				getch();
				if (ch == '=')
				{
					sym = geq;
					getch();
				}
				else
				{
					sym = gtr;
				}
			}
			else
			{
				sym = ssym[ch];		/* �����Ų�������������ʱ��ȫ�����յ��ַ����Ŵ��� */
				if (ch != EOF)     /* ���û�е����ļ�ĩβ��������ȡ��һ���ַ� */
				{
					getch();
				}
			}
		}
	}
}

/*
 * �������������
 *
 * x: instruction.f;
 * y: instruction.l;
 * z: instruction.a;
 * s: instruction.s;
 */
void gen(enum fct x, int y, int z, std::string s)
{
	if (cx >= cxmax)
	{
		printf("Program is too long!\n");	/* ���ɵ���������������� */
		exit(1);
	}
	if (z >= amax)
	{
		printf("Displacement address is too big!\n");	/* ��ַƫ��Խ�� */
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	code[cx].s = s;
	cx++;
}


/*
 * ���Ե�ǰ�����Ƿ�Ϸ�
 *
 * ���﷨�����������ںͳ��ڴ����ò��Ժ���test��
 * ��鵱ǰ���ʽ�����˳����﷨��λ�ĺϷ���
 *
 * s1:	��Ҫ�ĵ��ʼ���
 * s2:	���������Ҫ�ĵ��ʣ���ĳһ����״̬ʱ��
 *      �ɻָ��﷨�����������������Ĳ��䵥�ʷ��ż���
 * n:  	�����
 */
void test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))
	{
		error(n);
		/* ����ⲻͨ��ʱ����ͣ��ȡ���ţ�ֱ����������Ҫ�ļ��ϻ򲹾ȵļ��� */
		while ((!inset(sym, s1)) && (!inset(sym, s2)))
		{
			getsym();
		}
	}
}

/*
 * �����������
 *
 * lev:    ��ǰ�ֳ������ڲ�
 * tx:     ���ű�ǰβָ��
 * fsys:   ��ǰģ���̷��ż��ϡ�follow_symbol����ǰ�������Խ��ܵĺ�̷��ż���
 */


void block(int lev, int tx, bool* fsys)
{
	//L24���ķ�������û��const��procedure���������е�����б��������Ű�Χ��
	int i;

	int dx;                 /* ��¼���ݷ������Ե�ַ */
	int tx0;                /* ������ʼtx */
	int cx0;                /* ������ʼcx */
	bool nxtlev[symnum];    /* ���¼������Ĳ����У����ż��Ͼ�Ϊֵ�Σ�������ʹ������ʵ�֣�
							   ���ݽ�������ָ�룬Ϊ��ֹ�¼������ı��ϼ������ļ��ϣ������µĿռ�
							   ���ݸ��¼�����*/

	dx = 3;                 /* �����ռ����ڴ�ž�̬��SL����̬��DL�ͷ��ص�ַRA  */
	tx0 = tx;		        /* ��¼�����ʶ���ĳ�ʼλ�� */
	table[tx].adr = cx;	    /* ��¼��ǰ�����Ŀ�ʼλ�� */
	gen(jmp, 0, 0);         /* ������תָ���תλ��δ֪��ʱ��0 */

	do
	{
		if (sym == varsym)		//����������� var a=28, c= 29;ͬPL0
		{
			getsym();
			do {
				vardeclaration(&tx, lev, &dx);
				while (sym == comma)
				{
					getsym();
					vardeclaration(&tx, lev, &dx);
				}
				if (sym == semicolon)
				{
					getsym();
					break;
				}
				else
				{
					error(5); /* ©���˷ֺ� */
				}
			} while (sym == ident);
		}
		if (sym == strsym)    //�����ַ�����������
		{
			getsym();
			do {
				strdeclaration(&tx, lev, &dx);
				while (sym == comma)
				{
					getsym();
					strdeclaration(&tx, lev, &dx);
				}
				if (sym == semicolon)
				{
					getsym();
					break;
				}
				else
				{
					error(5); /* ©���˷ֺ� */
				}
			} while (sym == ident);
		}
		memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
		nxtlev[ident] = true;
		test(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));	/* ֱ��û���������� */

	
	code[table[tx0].adr].a = cx;	/* ��ǰ�����ɵ���ת������תλ�øĳɵ�ǰλ�� */
	table[tx0].adr = cx;	        /* ��¼��ǰ���̴����ַ */
	table[tx0].size = dx;	        /* ����������ÿ����һ�����������dx����1�����������Ѿ�������dx���ǵ�ǰ�������ݵ�size */
	cx0 = cx;
	gen(ini, 0, dx);	            /* ����ָ���ָ��ִ��ʱ������ջ��Ϊ�����õĹ��̿���dx����Ԫ�������� */

	if (tableswitch)		/* ������ű� */
	{
		for (i = 1; i <= tx; i++)
		{
			switch (table[i].kind)
			{
			case variable:
				printf("    %d var   %s ", i, table[i].name);
				printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
				fprintf(ftable, "    %d var   %s ", i, table[i].name);
				fprintf(ftable, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			case stringvar:
				printf("    %d str  %s ", i, table[i].name);
				printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				fprintf(ftable, "    %d str  %s ", i, table[i].name);
				fprintf(ftable, "lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				break;
			}
		}
		printf("\n");
		fprintf(ftable, "\n");
	}

	
	memcpy(nxtlev, fsys, sizeof(bool)* symnum);	/* ÿ����̷��ż��϶������ϲ��̷��ż��ϣ��Ա㲹�� */
	nxtlev[semicolon] = true;			//L24������̷�������;
	statementlist(nxtlev, &tx, lev);
	gen(opr, 0, 0);	                    /* ÿ�����̳��ڶ�Ҫʹ�õ��ͷ����ݶ�ָ�� */
	memset(nxtlev, 0, sizeof(bool)* symnum);	/* �ֳ���û�в��ȼ��� */
	test(fsys, nxtlev, 8);            	/* ����̷�����ȷ�� */
	listcode(cx0);                      /* ������ֳ������ɵĴ��� */
}

/*
 * �ڷ��ű��м���һ��
 *
 * k:      ��ʶ��������Ϊconst��var��procedure
 * ptx:    ���ű�βָ���ָ�룬Ϊ�˿��Ըı���ű�βָ���ֵ
 * lev:    ��ʶ�����ڵĲ��
 * pdx:    dxΪ��ǰӦ����ı�������Ե�ַ�������Ҫ����1
 *
 */
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* ���ű��name���¼��ʶ�������� */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case variable:	/* ���� */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case stringvar:	/* ���� */
		table[(*ptx)].level = lev;  // ��¼�ַ������ڵĲ�Ρ�
		table[(*ptx)].adr = (*pdx);  // ��¼�ַ�������Ե�ַ��
		(*pdx)++;  // ���ݷ����ַ������
		break;
	}
}

/*
 * ���ұ�ʶ���ڷ��ű��е�λ�ã���tx��ʼ������ұ�ʶ��
 * �ҵ��򷵻��ڷ��ű��е�λ�ã����򷵻�0
 *
 * id:    Ҫ���ҵ�����
 * tx:    ��ǰ���ű�βָ��
 */
int position(char* id, int tx)
{
	int i;
	strcpy(table[0].name, id);
	i = tx;
	while (strcmp(table[i].name, id) != 0)
	{
		i--;
	}
	return i;
}


/*
 * ������������
 */
void vardeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		enter(variable, ptx, lev, pdx);	// ��д���ű�
		getsym();
	}
	else
		error(4);    /* var��str��Ӧ�Ǳ�ʶ�� */
}

//�ַ�����������
void strdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		enter(stringvar, ptx, lev, pdx);
		getsym();
	}
	else
		error(4);    /* var��str��Ӧ�Ǳ�ʶ�� */
}

/*
 * ���Ŀ������嵥
 */
void listcode(int cx0)
{
	int i;
	if (listswitch)
	{
		printf("\n");
		for (i = cx0; i < cx; i++)
		{
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

/*
 * �������Ŀ�����
 */
void listall()
{
	int i;
	if (listswitch)
	{
		for (i = 0; i < cx; i++)
		{
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
			fprintf(fcode, "%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

//�ж�stmt;�ṹ�����жϻ����ţ�����������main��if��while�����ж�
void statementlist(bool* fsys, int* ptx, int lev) 
{
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[semicolon] = true;

	statement(nxtlev, ptx, lev);

	// ѭ������ <stmt> ;��ֱ����һ�����Ų�����俪ʼ����
	while (inset(sym, statbegsys) || sym == semicolon) 
	{
		if (sym == semicolon) 
		{
			getsym();
		}
		else 
		{
			error(10);  // ȱ�ٷֺ�
		}
		statement(nxtlev, ptx, lev);
	}
}


/*
 * ��䴦��
 */
void statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];

	if (sym == ident)	/* ׼�����ո�ֵ��䴦�� */
	{
		i = position(id, *ptx);/* ���ұ�ʶ���ڷ��ű��е�λ�� */
		if (i == 0)
		{
			error(11);	/* ��ʶ��δ���� */
		}
		else
		{
			if (table[i].kind != variable && table[i].kind != stringvar)
			{
				error(12);    /* ��ֵ����У���ֵ���󲿱�ʶ��Ӧ���Ǳ������ַ������� */
				i = 0;
			}
			else
			{
				getsym();
				if (sym == becomes)
				{
					getsym();
				}
				else
				{
					error(13);    /* û�м�⵽��ֵ���� */
				}
				memcpy(nxtlev, fsys, sizeof(bool) * symnum);
				//expression(nxtlev, ptx, lev);    /* ����ֵ�����Ҳ���ʽ */
				if (table[i].kind == variable)
				{
					expression(nxtlev, ptx, lev);    /* ����ֵ�����Ҳ���ʽ */
				}
				else if (table[i].kind == stringvar)
				{
					strexpr(nxtlev, ptx, lev);    /* �����ַ�����ֵ */
				}

				if (i != 0)
				{
					if (table[i].kind == variable)
					{
						gen(sto, lev - table[i].level, table[i].adr);
					}
					else if (table[i].kind == stringvar)
					{
						gen(sto, lev - table[i].level, table[i].adr, str); /* �����ַ�����ֵ */
					}
				}
			}
		}
	}
	else
	{
		if (sym == scansym)	/* ׼������read��䴦�� */
		{
			getsym();
			if (sym != lparen)
			{
				error(34);	/* ��ʽ����Ӧ�������� */
			}
			else
			{
				do {
					getsym();
					if (sym == ident)
					{
						i = position(id, *ptx);	/* ����Ҫ���ı��� */
					}
					else
					{
						i = 0;
					}

					if (i == 0)
					{
						error(35);	/* read��������еı�ʶ��Ӧ�����������ı��� */
					}
					else
					{
						if (table[i].kind == variable)
						{
							gen(opr, 0, 16);  /* ��������ָ���ȡֵ��ջ�� */
						}
						else if (table[i].kind == stringvar)
						{
							gen(opr, 0, 17);  /* ���������ַ���ָ�� */
						}
						gen(sto, lev - table[i].level, table[i].adr);  /* ��ջ���������������Ԫ�� */
					}
					getsym();

				} while (sym == comma);	/* һ��read���ɶ�������� */
			}
			if (sym != rparen)
			{
				error(33);	/* ��ʽ����Ӧ�������� */
				while (!inset(sym, fsys))	/* �����ȣ�ֱ�������ϲ㺯���ĺ�̷��� */
				{
					getsym();
				}
			}
			else
			{
				getsym();
			}
		}
		else if (sym == printsym)    /* ׼������print��䴦�� */
		{
			getsym();
			if (sym == lparen)
			{
				do {
					getsym();
					memcpy(nxtlev, fsys, sizeof(bool) * symnum);
					nxtlev[rparen] = true;
					nxtlev[comma] = true;
					if (sym == conststr)
					{
						strexpr(nxtlev, ptx, lev);
						gen(lits, 0, 0, str);  /* ���ַ���������ջ */
						gen(opr, 0, 21);       /* ��������ַ�����ָ�� */
					}
					else
					{
						
						if (table[position(id, *ptx)].kind == stringvar)
						{
							strexpr(nxtlev, ptx, lev);
							gen(opr, 0, 21);   /* ��������ַ�����ָ�� */
						}
						else
						{
							expression(nxtlev, ptx, lev);  /* ���ñ��ʽ���� */
							gen(opr, 0, 14);   /* �������������ָ�� */
						}
					}
					gen(opr, 0, 15);           /* ���ɻ���ָ�� */
				} while (sym == comma);        /* һ��print��������������ֵ */
				if (sym != rparen)
				{
					error(33);  /* ��ʽ����Ӧ�������� */
				}
				else
				{
					getsym();
				}
			}
		}

		/* ׼������if��䴦��
		if (bool_expr) then {stmt_list} end
		if (bool_expr) then {stmt_list} else {stmt_list} end
		*/
		if (sym == ifsym)  // ���� if ���
		{
			getsym();
			if (sym != lparen)
			{
				error(6);  // ȱ��������
			}
			else
			{
				getsym();
				memcpy(nxtlev, fsys, sizeof(bool) * symnum);
				nxtlev[rparen] = true;
				condition(nxtlev, ptx, lev);  // �����������ʽ
				if (sym == rparen)
				{
					getsym();
					if (sym == thensym)
					{
						getsym();
						if (sym != lbrace)
						{
							error(7);  // ȱ��������
						}
						else
						{
							getsym();
							cx1 = cx;  // ���浱ǰָ���ַ
							gen(jpc, 0, 0);  // ����������תָ���ת��ַδ֪����ʱд0

							memcpy(nxtlev, fsys, sizeof(bool) * symnum);
							nxtlev[rbrace] = true;
							statementlist(nxtlev, ptx, lev);  // ���� then ���ֵ�����б�
							code[cx1].a = cx;  // ������ת��ַ

							if (sym == rbrace)
							{
								getsym();
								if (sym == elsesym)
								{
									getsym();
									if (sym != lbrace)
									{
										error(7);  // ȱ��������
									}
									else
									{
										getsym();
										cx2 = cx;  // ���浱ǰָ���ַ
										gen(jmp, 0, 0);  // ������������תָ���ת��ַδ֪����ʱд0
										code[cx1].a = cx;  // ������ת��ַ

										memcpy(nxtlev, fsys, sizeof(bool) * symnum);
										nxtlev[rbrace] = true;
										statementlist(nxtlev, ptx, lev);  // ���� else ���ֵ�����б�
										code[cx2].a = cx;  // ������ת��ַ

										if (sym == rbrace)
										{
											getsym();
											if (sym != endsym)
											{
												error(6);  // ȱ�� end
											}
											else
											{
												getsym();
											}
										}
										else
										{
											error(6);  // ȱ���һ�����
										}
									}
								}
								else
								{
									if (sym != endsym)
									{
										error(6);  // ȱ�� end
									}
									else
									{
										getsym();
									}
								}
							}
							else
							{
								error(6);  // ȱ���һ�����
							}
						}
					}
					else
					{
						error(16);  // ȱ�� then
					}
				}
				else
				{
					myerr("ȱ��)");
				}
			}
		}
		else if (sym == whilesym)	/* ׼������while��䴦�� */
		{
			cx1 = cx;	/* �����ж�����������λ�� */
			getsym();
			if (sym != lparen)
			{
				myerr("ȱ��(");
			}
			else
			{
				getsym();
				memcpy(nxtlev, fsys, sizeof(bool) * symnum);
				nxtlev[rparen] = true;
				condition(nxtlev, ptx, lev);
				if (sym == rparen)
				{
					getsym();
					if (sym == lbrace)	// {
					{
						getsym();
						cx2 = cx; /* ����ѭ����Ľ�������һ��λ�� */
						gen(jpc, 0, 0); /* ����������ת��������ѭ���ĵ�ַδ֪�����Ϊ0�ȴ����� */
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						nxtlev[rbrace] = true;
						statementlist(nxtlev, ptx, lev); /* ��������б� */
						gen(jmp, 0, cx1); /* ����������תָ���ת��ǰ���ж�����������λ�� */
						code[cx2].a = cx; /* ��������ѭ���ĵ�ַ */
						if (sym == rbrace)
						{
							getsym();
						}
						else
						{
							error(33); /* ��ʽ����Ӧ���һ����� */
						}
					}
					else
					{
						error(34); /* ��ʽ����Ӧ�������� */
					}
				}
				else
				{
					myerr("ȱ��)");
				}
			}
		}

	}
	memset(nxtlev, 0, sizeof(bool) * symnum);	/* �������޲��ȼ��� */
	test(fsys, nxtlev, 19);	/* �������������ȷ�� */
}

/*
 * ���ʽ����
 */
void expression(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;	/* ���ڱ��������� */
	bool nxtlev[symnum];

	if (sym == plus || sym == minus)	/* ���ʽ��ͷ�������ţ���ʱ��ǰ���ʽ������һ�����Ļ򸺵��� */
	{
		addop = sym;	/* ���濪ͷ�������� */
		getsym();
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* ������ */
		if (addop == minus)
		{
			gen(opr, 0, 1);	/* �����ͷΪ��������ȡ��ָ�� */
		}
	}
	else	/* ��ʱ���ʽ��������ļӼ� */
	{
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* ������ */
	}
	while (sym == plus || sym == minus)
	{
		addop = sym;
		getsym();
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* ������ */
		if (addop == plus)
		{
			gen(opr, 0, 2);	/* ���ɼӷ�ָ�� */
		}
		else
		{
			gen(opr, 0, 3);	/* ���ɼ���ָ�� */
		}
	}
}

/*
 * ���
 */
void term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;	/* ���ڱ���˳������� */
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	factor(nxtlev, ptx, lev);	/* �������� */
	while (sym == times || sym == slash)
	{
		mulop = sym;
		getsym();
		factor(nxtlev, ptx, lev);
		if (mulop == times)
		{
			gen(opr, 0, 4);	/* ���ɳ˷�ָ�� */
		}
		else
		{
			gen(opr, 0, 5);	/* ���ɳ���ָ�� */
		}
	}
}

/*
 * ���Ӵ���
 */
void factor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	test(facbegsys, fsys, 24);	/* ������ӵĿ�ʼ���� */
	while (inset(sym, facbegsys)) 	/* ѭ���������� */
	{
		if (sym == ident)	/* ����Ϊ��ʶ�� */
		{
			i = position(id, *ptx);	/* ���ұ�ʶ���ڷ��ű��е�λ�� */
			if (i == 0)
			{
				error(11);	/* ��ʶ��δ���� */
			}
			else
			{
				switch (table[i].kind)
				{
				case variable:	/* ��ʶ��Ϊ���� */
					gen(lod, lev - table[i].level, table[i].adr);	/* �ҵ�������ַ������ֵ��ջ */
					break;
				default:
					error(21);	/* ��ʶ�����ʹ��� */
					break;
				}
			}
			getsym();
		}
		else if (sym == number)	/* ����Ϊ�� */
		{
			if (num > amax)
			{
				error(31); /* ��Խ�� */
				num = 0;
			}
			gen(lit, 0, num);
			getsym();
		}
		else if (sym == lparen)	/* ����Ϊ���ʽ */
		{
			getsym();
			memcpy(nxtlev, fsys, sizeof(bool) * symnum);
			nxtlev[rparen] = true;
			expression(nxtlev, ptx, lev);
			if (sym == rparen)
			{
				getsym();
			}
			else
			{
				error(22);	/* ȱ�������� */
			}
		}
		else
		{
			error(24);	/* ��Ч������ */
		}
		memset(nxtlev, 0, sizeof(bool) * symnum);
		nxtlev[lparen] = true;
		test(fsys, nxtlev, 23); /* һ�����Ӵ�����ϣ������ĵ���Ӧ��fsys������ */
		/* ������ǣ������ҵ���һ�����ӵĿ�ʼ��ʹ�﷨�������Լ���������ȥ */
	}
}


//�κζ����ģ����磨���ڵģ��ַ����ʽ���������ַ�����Ϊ��ͷ
void strexpr(bool* fsys, int* ptx, int lev)
{
	bool nxtlev[symnum];
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[plus] = true;
	strterm(nxtlev, ptx, lev);  // ����strterm
	while (sym == plus)
	{
		getsym();

		if (sym == ident)
		{
			int i = position(id, *ptx);
			if (i == 0)
			{
				error(11);  // ��ʶ��δ����
			}
			else if (table[i].kind == variable)
			{
				strterm(nxtlev, ptx, lev);
				gen(opr, 0, 20);  // �����ַ������������ָ��
			}
			else
			{
				strterm(nxtlev, ptx, lev);  // ����strterm
				gen(opr, 0, 18);  // �����ַ������ָ��
			}
		}
		else
		{
			strterm(nxtlev, ptx, lev);  // ����strterm
			gen(opr, 0, 18);  // �����ַ������ָ��
		}
	}
}




void strterm(bool* fsys, int* ptx, int lev)
{
	bool nxtlev[symnum];
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[times] = true;
	strfactor(nxtlev, ptx, lev);  // ����strfactor
	while (sym == times)
	{
		getsym();
		
		factor(nxtlev, ptx, lev);  // ����factor
		gen(opr, 0, 19);  // �����ַ����˷�ָ��
	}
}

void strfactor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	if (sym == ident)
	{
		i = position(id, *ptx);
		if (i == 0)
		{
			error(11);  // ��ʶ��δ����
		}
		else
		{
			switch (table[i].kind)
			{
			case variable:
				gen(lod, lev - table[i].level, table[i].adr);
				break;
			case stringvar:
				gen(lod, lev - table[i].level, table[i].adr);
				break;
			default:
				error(21);  // ����Ϊ����
				break;
			}
		}
		getsym();
	}
	else if (sym == conststr)
	{
		gen(lits, 0, 0, str);  // �����ַ���������ָ��
		getsym();
	}
	else if (sym == number)
	{
		gen(lits, 0, 0,std::to_string(num));  // �������ֳ�����ָ��
		getsym();
	}
	else if (sym == lparen)
	{
		getsym();
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[rparen] = true;
		strexpr(nxtlev, ptx, lev);
		if (sym == rparen)
		{
			getsym();
		}
		else
		{
			error(22);  // ȱ��������
		}
	}
	else
	{
		error(24);  // ���������
	}
	memset(nxtlev, 0, sizeof(bool) * symnum);
	nxtlev[lparen] = true;
	test(fsys, nxtlev, 23);  // һ�����Ӵ�����ϣ������ĵ���Ӧ��fsys������
}

/*
 * ��������
 */
void condition(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	// L24��֧��odd����������ʡ��odd����
	
	/* �߼����ʽ���� */
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[eql] = true;
	nxtlev[neq] = true;
	nxtlev[lss] = true;
	nxtlev[leq] = true;
	nxtlev[gtr] = true;
	nxtlev[geq] = true;
	expression(nxtlev, ptx, lev);
	if (sym != eql && sym != neq && sym != lss && sym != leq && sym != gtr && sym != geq)
	{
		error(20); /* Ӧ��Ϊ��ϵ����� */
	}
	else
	{
		relop = sym;
		getsym();
		expression(fsys, ptx, lev);
		switch (relop)
		{
		case eql:
			gen(opr, 0, 8);
			break;
		case neq:
			gen(opr, 0, 9);
			break;
		case lss:
			gen(opr, 0, 10);
			break;
		case geq:
			gen(opr, 0, 11);
			break;
		case gtr:
			gen(opr, 0, 12);
			break;
		case leq:
			gen(opr, 0, 13);
			break;
		}
	}
}


/*
 * ���ͳ���
 */
void interpret()
{
	int p = 0; /* ָ��ָ�� */
	int b = 1; /* ָ���ַ */
	int t = 0; /* ջ��ָ�� */
	struct instruction i;	/* ��ŵ�ǰָ�� */
	int s[stacksize];	/* ջ */
	std::string str_s[stacksize * 3];
	int is = 0;

	printf("Start pl0\n");
	fprintf(fresult, "Start pl0\n");
	s[0] = 0; /* s[0]���� */
	s[1] = 0; /* �������������ϵ��Ԫ����Ϊ0 */
	s[2] = 0;
	s[3] = 0;
	do {
		i = code[p];	/* ����ǰָ�� */
		p = p + 1;
		switch (i.f)
		{
		case lit:	/* ������a��ֵȡ��ջ�� */
			t = t + 1;
			s[t] = i.a;
			break;
		case lits:		//��instruction.s�ַ�����str_sջ��������������.s������is�ŵ�s[]ջ��
			//�Ȱ�i.s�浽str_s�Ȼ������is++����string������ȡ��ջ��
			t = t + 1;
			s[t] = is;
			str_s[is] = i.s;
			is++;		//ָ����һ��λ��
			break;
		case opr:	/* ��ѧ���߼����� */
			switch (i.a)
			{
			case 0:  /* �������ý����󷵻� */
				t = b - 1;
				p = s[t + 3];
				b = s[t + 2];
				break;
			case 1: /* ջ��Ԫ��ȡ�� */
				s[t] = -s[t];
				break;
			case 2: /* ��ջ�������ջ���������ջԪ�أ����ֵ��ջ */
				t = t - 1;
				s[t] = s[t] + s[t + 1];
				break;
			case 3:/* ��ջ�����ȥջ���� */
				t = t - 1;
				s[t] = s[t] - s[t + 1];
				break;
			case 4:/* ��ջ�������ջ���� */
				t = t - 1;
				s[t] = s[t] * s[t + 1];
				break;
			case 5:/* ��ջ�������ջ���� */
				t = t - 1;
				s[t] = s[t] / s[t + 1];
				break;
			case 6:/* ջ��Ԫ�ص���ż�ж� */
				s[t] = s[t] % 2;
				break;
			case 8:/* ��ջ������ջ�����Ƿ���� */
				t = t - 1;
				s[t] = (s[t] == s[t + 1]);
				break;
			case 9:/* ��ջ������ջ�����Ƿ񲻵� */
				t = t - 1;
				s[t] = (s[t] != s[t + 1]);
				break;
			case 10:/* ��ջ�����Ƿ�С��ջ���� */
				t = t - 1;
				s[t] = (s[t] < s[t + 1]);
				break;
			case 11:/* ��ջ�����Ƿ���ڵ���ջ���� */
				t = t - 1;
				s[t] = (s[t] >= s[t + 1]);
				break;
			case 12:/* ��ջ�����Ƿ����ջ���� */
				t = t - 1;
				s[t] = (s[t] > s[t + 1]);
				break;
			case 13: /* ��ջ�����Ƿ�С�ڵ���ջ���� */
				t = t - 1;
				s[t] = (s[t] <= s[t + 1]);
				break;
			case 14:/* ջ��ֵ��� */
				printf("%d", s[t]);
				fprintf(fresult, "%d", s[t]);
				t = t - 1;
				break;
			case 15:/* ������з� */
				printf("\n");
				fprintf(fresult, "\n");
				break;
			case 16:/* ����һ����������ջ�� */
				t = t + 1;
				printf("input: ");
				fprintf(fresult, "input: ");
				scanf("%d", &(s[t]));
				fprintf(fresult, "%d\n", s[t]);
				break;
			case 17:	//����һ���ַ�������ջ��
				t = t + 1;
				printf("input: ");
				fprintf(fresult, "input: ");
				std::cin >> str_s[is];
				s[t] = is;			//ջ�����������ַ�����str_s�е��±�
				is++;
				break;
			case 18:	//�ַ���+�ַ���
				t = t - 1;
				str_s[is] = str_s[s[t]] + str_s[s[t + 1]];
				s[t] = is;
				is++;
				break;
			case 19:	//�ַ��� * n
				t = t - 1;
				str_s[is] = scopy(str_s[s[t]], s[t + 1]);
				s[t] = is;
				is++;
				break;
			case 20:	//�ַ���+����num
				t = t - 1;
				str_s[is] = str_s[s[t]] + std::to_string(s[t + 1]);
				s[t] = is;
				is++;
				break;
			case 21:    // ����ַ���
				std::cout << str_s[s[t]] << std::endl;
				t = t - 1;
				break;
			}
			break;
		case lod:	/* ȡ��Ե�ǰ���̵����ݻ���ַΪa���ڴ��ֵ��ջ�� */
			t = t + 1;
			s[t] = s[base(i.l, s, b) + i.a];
			break;
		case sto:	/* ջ����ֵ�浽��Ե�ǰ���̵����ݻ���ַΪa���ڴ� */
			s[base(i.l, s, b) + i.a] = s[t];
			t = t - 1;
			break;
		case ini:	/* ������ջ��Ϊ�����õĹ��̿���a����Ԫ�������� */
			t = t + i.a;
			break;
		case jmp:	/* ֱ����ת */
			p = i.a;
			break;
		case jpc:	/* ������ת */
			if (s[t] == 0)
				p = i.a;
			t = t - 1;
			break;
		}
	} while (p != 0);
	printf("End pl0\n");
	fprintf(fresult, "End pl0\n");
}

/* ͨ�����̻�ַ����l����̵Ļ�ַ */
int base(int l, int* s, int b)
{
	int b1;
	b1 = b;
	while (l > 0)
	{
		b1 = s[b1];
		l--;
	}
	return b1;
}

