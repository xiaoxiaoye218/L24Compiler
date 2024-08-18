/*
 * PL/0 complier program implemented in C
 * PL/0编译器（完整版本）
 * The program has been tested on Visual Studio 2022
 *
 * 使用方法：
 * 运行后输入PL/0源程序文件名
 * 回答是否输出虚拟机代码
 * 回答是否输出符号表
 * fcode.txt输出虚拟机代码
 * foutput.txt输出源文件、出错示意（如有错）和各行对应的生成代码首地址（如无错）
 * fresult.txt输出运行结果
 * ftable.txt输出符号表
 */
#define  _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>


#define bool int
#define true 1
#define false 0

#define norw 10       /* 保留字个数 */
#define txmax 100     /* 符号表容量 */
#define nmax 14       /* 数字的最大位数 */
#define al 10         /* 标识符的最大长度 */
#define maxerr 30     /* 允许的最多错误数 */
#define amax 2048     /* 地址上界*/
#define levmax 3      /* 最大允许过程嵌套声明层数*/
#define cxmax 200     /* 最多的虚拟机代码数 */
#define stacksize 500 /* 运行时数据栈元素最多为500个 */

 /* 符号 */
//保留字（13 个）标识符（ident）数字（number）运算符和分隔符（例如 plus, minus, times, slash, eql, neq, lss, leq, gtr, geq, lparen, rparen, comma, semicolon, period, becomes）特殊符号（nul）
enum symbol {
	nul, ident, number,	conststr,			//标识符、数字、""字符串	var--number, strsym--conststr
	plus, minus, times, slash,				//+、-、*、/
	becomes, eql, neq,lss, leq, gtr, geq,	//=, ==, !=, <, <=, >, >=
	lparen, rparen, lbrace, rbrace,			//(, ), {, }
	comma, semicolon, 						//, ; 
	ifsym, thensym, elsesym, endsym,		//if, then, else, endd
	whilesym,								//while
	printsym, scansym,						//原型是writesym, readsym, 
	varsym, strsym,							//无了constsym、procsym，加了strsym
	mainsym, 
};
#define symnum 31

/* 符号表中的类型 */
enum object 
{
	variable,
	stringvar,
};

/* 虚拟机代码指令 */
enum fct {
	lit, opr, lod,
	sto, cal, ini,
	jmp, jpc,
	lits, lods, stos
};
#define fctnum 8			//指令的个数

/* 虚拟机代码结构 */
struct instruction
{
	enum fct f; /* 虚拟机代码指令 */
	int l;      /* 引用层与声明层的层次差 */
	int a;      /* 根据f的不同而不同 */
	std::string s;
};


bool listswitch;   /* 显示虚拟机代码与否 */
bool tableswitch;  /* 显示符号表与否 */
char ch;            /* 存放当前读取的字符，getch 使用 */
enum symbol sym;    /* 当前的符号 */
char id[al + 1];      /* 当前ident，多出的一个字节用于存放0 */
int num;            /* 当前number */
std::string str;	
int cc, ll;         /* getch使用的计数器，cc表示当前字符(ch)的位置 */
int cx;             /* 虚拟机代码指针, 取值范围[0, cxmax-1]*/
char line[81];      /* 读取行缓冲区 */
char a[al + 1];       /* 临时符号，多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
char word[norw][al];        /* 保留字 */					//[保留字个数][保留字最大长度]，在init中，是按字典序升序存储的，方便二分查找
enum symbol wsym[norw];     /* 保留字对应的符号值 */		//建立保留字在word中的索引 与 enum symbol中的值的对应关系
enum symbol ssym[256];      /* 单字符的符号值 */
char mnemonic[fctnum][5];   /* 虚拟机代码指令名称 */
bool declbegsys[symnum];    /* 表示声明开始的符号集合 */	//常量声明，变量声明，过程声明
bool statbegsys[symnum];    /* 表示语句开始的符号集合 */	//赋值语句、条件语句、循环语句
bool facbegsys[symnum];     /* 表示因子开始的符号集合 */	//标识符、数字、括号表达式
bool strfacbegsys[symnum];

/* 符号表结构 */
struct tablestruct
{
	char name[al];	    /* 名字 */
	enum object kind;	/* 类型：constant，variable或procedure */
	int val;            /* 数值，仅const使用 */
	int level;          /* 所处层，仅const不使用 */
	int adr;            /* 地址，仅const不使用 */
	int size;           /* 需要分配的数据区空间, 仅procedure使用 */
};

struct tablestruct table[txmax]; /* 符号表 */

FILE* fin;      /* 输入源文件 */
FILE* ftable;	/* 输出符号表 */
FILE* fcode;    /* 输出虚拟机代码 */
FILE* foutput;  /* 输出文件及出错示意（如有错）、各行对应的生成代码首地址（如无错） */
FILE* fresult;  /* 输出执行结果 */
char fname[al];
int err;        /* 错误计数器 */

void error(int n);		//打印错误信息并技术错误数量，n：错误编号
void getsym();			//symbol，进行词法分析，将当前读取的字符转换为符号
void getch();			//读取下一个字符，并处理换行和空白符
void init();	//初始化各类表格、集合和符号
void gen(enum fct x, int y, int z,std::string="");			//生成虚拟机代码
void test(bool* s1, bool* s2, int n);		//测试当前符号是否合法
int inset(int e, bool* s);		//检查元素是否在集合中
int addset(bool* sr, bool* s1, bool* s2, int n);	//计算两个集合的并集
int subset(bool* sr, bool* s1, bool* s2, int n);	//差集
int mulset(bool* sr, bool* s1, bool* s2, int n);	//交集
void block(int lev, int tx, bool* fsys);	//处理分程序
void interpret();									//解释执行虚拟机代码
void factor(bool* fsys, int* ptx, int lev);			//处理因子
void term(bool* fsys, int* ptx, int lev);			//处理项
void condition(bool* fsys, int* ptx, int lev);		//处理条件
void expression(bool* fsys, int* ptx, int lev);		//处理表达式
void statement(bool* fsys, int* ptx, int lev);		//处理语句
void listcode(int cx0);									//输出目标代码
void listall();											//输出所有目标代码
void vardeclaration(int* ptx, int lev, int* pdx);		//处理变量声明
void constdeclaration(int* ptx, int lev, int* pdx);		//处理常量声明
int position(char* idt, int tx);				//查找标识符在符号表中的位置
void enter(enum object k, int* ptx, int lev, int* pdx);		//在符号表中加入一项
int base(int l, int* s, int b);							//通过过程基址求上一层过程的基址
void myerr(const char*);
void statementlist(bool* fsys, int* ptx, int lev);
void strdeclaration(int* ptx, int lev, int* pdx);
void strfactor(bool* fsys, int* ptx, int lev);			
void strterm(bool* fsys, int* ptx, int lev);			
void strcondition(bool* fsys, int* ptx, int lev);		
void strexpr(bool* fsys, int* ptx, int lev);		


#include <sstream>

// 将整数转换为字符串的辅助函数
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

/* 主程序开始 */
int main()
{
	bool nxtlev[symnum];

	printf("Input pl/0 file?   ");
	scanf("%s", fname);		/* 输入文件名 */

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
	rewind(fin);		//重拨指针到文件开头

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

	printf("List object codes?(Y/N)");	/* 是否输出虚拟机代码 */
	scanf("%s", fname);
	listswitch = (fname[0] == 'y' || fname[0] == 'Y');

	printf("List symbol table?(Y/N)");	/* 是否输出符号表 */
	scanf("%s", fname);
	tableswitch = (fname[0] == 'y' || fname[0] == 'Y');


	//--------------------------------分界线-----------------------------------


	init();		/* 初始化 */
	err = 0;	//记录编译过程中发生的错误数量
	cc = ll = cx = 0;	//cc：字符计数器，ll：行长度，cx：代码指针
	ch = ' ';

	getsym();		//调用词法分析函数，从源文件中获取第一个符号token


	if (sym == mainsym)	//增加一段main{}识别逻辑
	{
		getsym();
		if (sym == lbrace)	//确保{
		{
			//进了main{后的逻辑和原本PL0相同
			//将声明开始符号和语句开始符号合并到nxtlev中
			//因为在解析过程中，需要识别可以开始声明和语句的符号，便于block函数识别和处理声明或语句的起始位置
			getsym();
			bool nxtlev[symnum];
			addset(nxtlev, declbegsys, statbegsys, symnum);
			nxtlev[rbrace] = true;

			block(0, 0, nxtlev);  /* 处理分程序 */
		}
		else
			myerr("缺少'{'");
	}
	else 
	{
		myerr("缺少'main'");
	}



	if (err == 0)		//如果没有任何错误
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

		listall();	 /* 输出所有代码 */
		fclose(fcode);

		interpret();	/* 调用解释执行程序 */
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
 * 初始化
 */
void init()
{
	int i;

	/* 设置单字符符号 */
	for (i = 0; i <= 255; i++)
	{
		ssym[i] = nul;
	}
	ssym['+'] = plus;		//建立一个单字符的哈希映射	ssym[ch]=ch的符号值
	ssym['-'] = minus;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['{'] = lbrace;
	ssym['}'] = rbrace;
	//ssym['='] = becomes;		//=和==,所以是和<一样的逻辑。同理!=，在PL0里是单字符#，但是在L24里是两个字符!=，要用类似<=的逻辑; =是赋值，==是eql
	ssym[','] = comma;
	ssym[';'] = semicolon;

	/* 设置保留字名字,按照字母顺序，便于二分查找 */
	//word[保留字个数][保留字最大长度]
	//后面这个字符是和EBNF里的相一致的，比如PL0里procedure是因为语法就是这样
	//所以这里scan就不能擅自改成scanf；然后字符串类型的声明就是str xxx-------------
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

	/* 设置保留字符号 */
	wsym[0] = elsesym;			//word表中第0个字符对应的enum symbol号是elsesym
	wsym[1] = endsym;
	wsym[2] = ifsym;
	wsym[3] = mainsym;
	wsym[4] = printsym;
	wsym[5] = scansym;
	wsym[6] = strsym;
	wsym[7] = thensym;
	wsym[8] = varsym;
	wsym[9] = whilesym;


	/* 设置指令名称 */
	//mnemonic是指令的助记符，把前面定义的enum fct用mnemonic与字符串对应起来
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[ini][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");
	strcpy(&(mnemonic[lits][0]), "lits");

	/* 设置符号集 */
	for (i = 0; i < symnum; i++)
	{
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/* 设置声明开始符号集 */
	declbegsys[varsym] = true;
	declbegsys[strsym] = true;
	//varsym可以作为声明的开始符号

	/* 设置语句开始符号集 */
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[scansym] = true;
	statbegsys[printsym] = true;

	//beginsym,callsym,ifsym,whilesym分别表示begin语句、call语句，if语句，while语句

	/* 设置因子开始符号集 */
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;

	strfacbegsys[ident] = true;
	strfacbegsys[number] = true;
	strfacbegsys[conststr] = true;
	strfacbegsys[lparen] = true;
	//ident,number,lparen 标识符、数字、左括号

	//用途：if当前符号属于声明开始符号集，则会进入相应的声明解析逻辑，处理常量声明、变量声明、过程声明
	//		if当前符号属于语句开始符号集，则会进入相应的语句解析逻辑 
}

/*
 * 用数组实现集合的集合运算
 */
int inset(int e, bool* s)
{
	return s[e];
}

//合并两个开始符号集，结果存储在目标集合sr中，n参数指定集合的大小，即符号的总数
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
 *	出错处理，打印出错位置和错误编码
 */
void error(int n)
{
	char space[81];
	memset(space, 32, 81);

	space[cc - 1] = 0; /* 出错时当前符号已经读完，所以cc-1 */

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

	space[cc - 1] = 0; /* 出错时当前符号已经读完，所以cc-1 */

	printf("**%s^%s\n", space, s);
	fprintf(foutput, "**%s^%s\n", space, s);

	err = err + 1;
	if (err > maxerr)
	{
		exit(1);
	}
}

/*
 * 过滤空格，读取一个字符
 * 每次读一行，存入line缓冲区，line被getsym取空后再读一行
 * 被函数getsym调用
 */
void getch()
{
	//cc:current character position当前字符的位置计数器，表示当前正在处理的字符在缓冲区中的下标位置
	//ll:line length 当前行的长度计数器
	//if(cc == ll) ：如果缓冲区内的字符都被处理完了，或者都为0也就是还没处理
	if (cc == ll) /* 判断缓冲区中是否有字符，若无字符，则读入下一行字符到缓冲区中 */
	{
		//feof()检测文件指针是否到达文件末尾
		if (feof(fin))
		{
			printf("Program is incomplete!\n");
			exit(1);
		}
		ll = 0;
		cc = 0;
		//cx：code index，在main中被初始化为0，指向当前指令的位置
		printf("%d ", cx);
		fprintf(foutput, "%d ", cx);
		ch = ' ';
		//不判断制表符是因为getch的目的是读取整行字符，直到遇到换行符，其他空白字符不会影响行的结束判断 
		while (ch != 10)
		{
			//从fin读取一个字符，存储到ch中，如果读取到文件末尾，fscanf返回EOF，
			if (EOF == fscanf(fin, "%c", &ch))
			{
				line[ll] = 0;
				break;
			}

			printf("%c", ch);
			fprintf(foutput, "%c", ch);
			line[ll] = ch;
			ll++;				//记录这一行的字符个数
		}
	}
	//这两行是从缓冲区line中获取当前字符，cc指向的是当前要读入的字符下表
	ch = line[cc];
	cc++;
}

/*
 * 词法分析，获取一个符号，并将其类型存储在全局变量sym中
 */
void getsym()
{
	int i, j, k;
	///* ch存放当前读取的字符，getch 使用 */
	while (ch == ' ' || ch == 10 || ch == 9)	/* 过滤空格、换行和制表符 */
	{
		getch();
	}
	//变量名、string名这些都可以称为标识符
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) /* 当前的单词是标识符或是保留字 */
	{
		k = 0;		//临时计数器
		//a[]是全局变量，用于存放临时符号
		do {
			if (k < al)
			{
				a[k] = ch;
				k++;
			}
			getch();
		} while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'));
		a[k] = 0;		//在数组末尾添加终止符
		strcpy(id, a);	//吧临时字符数组a[]复制到标识符数组id[]
		i = 0;
		j = norw - 1;		//norw是保留字word[]的数量，i，j其实就是二分查找的左右边界
		do 
		{    /* 搜索当前单词是否为保留字，使用二分法查找 */
			k = (i + j) / 2;
			//strcmp比较两个字符串，并返回一个整数，<=0：说明id<=work[k]
			if (strcmp(id, word[k]) <= 0)
			{
				j = k - 1;
			}
			if (strcmp(id, word[k]) >= 0)
			{
				i = k + 1;
			}
		} while (i <= j);
		if (i - 1 > j) /* 当前的单词是保留字 */
		{
			sym = wsym[k];		//k：是第几个保留字，wsym[k]记录了第k个保留字的enum值
		}
		else /* 当前的单词是标识符 */
		{
			sym = ident;		//ident本身就是enum symbol里的一个值
		}
	}
	//识别字符串
	else if (ch == '"')
	{
		//后面再加string类型常量的处理，类似number，不是var，var是声明而已
		str = "";		//重置str
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
			myerr("缺少\"");
	}
	else
	{
		if (ch >= '0' && ch <= '9') /* 当前的单词是数字 */
		{
			k = 0;
			num = 0;
			sym = number;
			do {
				num = 10 * num + ch - '0';
				k++;
				getch();;
			} while (ch >= '0' && ch <= '9'); /* 获取数字的值 */
			k--;
			if (k > nmax) /* 数字位数太多 */
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
					sym = nul;		//不能识别的符号
			}
			else if (ch == '<')		/* 检测小于或小于等于符号 */
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
			else if (ch == '>')		/* 检测大于或大于等于符号 */
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
				sym = ssym[ch];		/* 当符号不满足上述条件时，全部按照单字符符号处理 */
				if (ch != EOF)     /* 如果没有到达文件末尾，继续读取下一个字符 */
				{
					getch();
				}
			}
		}
	}
}

/*
 * 生成虚拟机代码
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
		printf("Program is too long!\n");	/* 生成的虚拟机代码程序过长 */
		exit(1);
	}
	if (z >= amax)
	{
		printf("Displacement address is too big!\n");	/* 地址偏移越界 */
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	code[cx].s = s;
	cx++;
}


/*
 * 测试当前符号是否合法
 *
 * 在语法分析程序的入口和出口处调用测试函数test，
 * 检查当前单词进入和退出该语法单位的合法性
 *
 * s1:	需要的单词集合
 * s2:	如果不是需要的单词，在某一出错状态时，
 *      可恢复语法分析继续正常工作的补充单词符号集合
 * n:  	错误号
 */
void test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))
	{
		error(n);
		/* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
		while ((!inset(sym, s1)) && (!inset(sym, s2)))
		{
			getsym();
		}
	}
}

/*
 * 编译程序主体
 *
 * lev:    当前分程序所在层
 * tx:     符号表当前尾指针
 * fsys:   当前模块后继符号集合。follow_symbol即当前代码块可以接受的后继符号集合
 */


void block(int lev, int tx, bool* fsys)
{
	//L24的文法规则中没有const和procedure，并且所有的语句列表都被花括号包围。
	int i;

	int dx;                 /* 记录数据分配的相对地址 */
	int tx0;                /* 保留初始tx */
	int cx0;                /* 保留初始cx */
	bool nxtlev[symnum];    /* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
							   传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空间
							   传递给下级函数*/

	dx = 3;                 /* 三个空间用于存放静态链SL、动态链DL和返回地址RA  */
	tx0 = tx;		        /* 记录本层标识符的初始位置 */
	table[tx].adr = cx;	    /* 记录当前层代码的开始位置 */
	gen(jmp, 0, 0);         /* 产生跳转指令，跳转位置未知暂时填0 */

	do
	{
		if (sym == varsym)		//处理变量声明 var a=28, c= 29;同PL0
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
					error(5); /* 漏掉了分号 */
				}
			} while (sym == ident);
		}
		if (sym == strsym)    //处理字符串变量声明
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
					error(5); /* 漏掉了分号 */
				}
			} while (sym == ident);
		}
		memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
		nxtlev[ident] = true;
		test(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));	/* 直到没有声明符号 */

	
	code[table[tx0].adr].a = cx;	/* 把前面生成的跳转语句的跳转位置改成当前位置 */
	table[tx0].adr = cx;	        /* 记录当前过程代码地址 */
	table[tx0].size = dx;	        /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
	cx0 = cx;
	gen(ini, 0, dx);	            /* 生成指令，此指令执行时在数据栈中为被调用的过程开辟dx个单元的数据区 */

	if (tableswitch)		/* 输出符号表 */
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

	
	memcpy(nxtlev, fsys, sizeof(bool)* symnum);	/* 每个后继符号集合都包含上层后继符号集合，以便补救 */
	nxtlev[semicolon] = true;			//L24中语句后继符必须是;
	statementlist(nxtlev, &tx, lev);
	gen(opr, 0, 0);	                    /* 每个过程出口都要使用的释放数据段指令 */
	memset(nxtlev, 0, sizeof(bool)* symnum);	/* 分程序没有补救集合 */
	test(fsys, nxtlev, 8);            	/* 检测后继符号正确性 */
	listcode(cx0);                      /* 输出本分程序生成的代码 */
}

/*
 * 在符号表中加入一项
 *
 * k:      标识符的种类为const，var或procedure
 * ptx:    符号表尾指针的指针，为了可以改变符号表尾指针的值
 * lev:    标识符所在的层次
 * pdx:    dx为当前应分配的变量的相对地址，分配后要增加1
 *
 */
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* 符号表的name域记录标识符的名字 */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case variable:	/* 变量 */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case stringvar:	/* 过程 */
		table[(*ptx)].level = lev;  // 记录字符串所在的层次。
		table[(*ptx)].adr = (*pdx);  // 记录字符串的相对地址。
		(*pdx)++;  // 数据分配地址递增。
		break;
	}
}

/*
 * 查找标识符在符号表中的位置，从tx开始倒序查找标识符
 * 找到则返回在符号表中的位置，否则返回0
 *
 * id:    要查找的名字
 * tx:    当前符号表尾指针
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
 * 变量声明处理
 */
void vardeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		enter(variable, ptx, lev, pdx);	// 填写符号表
		getsym();
	}
	else
		error(4);    /* var或str后应是标识符 */
}

//字符串声明处理
void strdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		enter(stringvar, ptx, lev, pdx);
		getsym();
	}
	else
		error(4);    /* var或str后应是标识符 */
}

/*
 * 输出目标代码清单
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
 * 输出所有目标代码
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

//判断stmt;结构，不判断花括号，花括号留给main、if、while单独判断
void statementlist(bool* fsys, int* ptx, int lev) 
{
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[semicolon] = true;

	statement(nxtlev, ptx, lev);

	// 循环解析 <stmt> ;，直到下一个符号不是语句开始符号
	while (inset(sym, statbegsys) || sym == semicolon) 
	{
		if (sym == semicolon) 
		{
			getsym();
		}
		else 
		{
			error(10);  // 缺少分号
		}
		statement(nxtlev, ptx, lev);
	}
}


/*
 * 语句处理
 */
void statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];

	if (sym == ident)	/* 准备按照赋值语句处理 */
	{
		i = position(id, *ptx);/* 查找标识符在符号表中的位置 */
		if (i == 0)
		{
			error(11);	/* 标识符未声明 */
		}
		else
		{
			if (table[i].kind != variable && table[i].kind != stringvar)
			{
				error(12);    /* 赋值语句中，赋值号左部标识符应该是变量或字符串变量 */
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
					error(13);    /* 没有检测到赋值符号 */
				}
				memcpy(nxtlev, fsys, sizeof(bool) * symnum);
				//expression(nxtlev, ptx, lev);    /* 处理赋值符号右侧表达式 */
				if (table[i].kind == variable)
				{
					expression(nxtlev, ptx, lev);    /* 处理赋值符号右侧表达式 */
				}
				else if (table[i].kind == stringvar)
				{
					strexpr(nxtlev, ptx, lev);    /* 处理字符串赋值 */
				}

				if (i != 0)
				{
					if (table[i].kind == variable)
					{
						gen(sto, lev - table[i].level, table[i].adr);
					}
					else if (table[i].kind == stringvar)
					{
						gen(sto, lev - table[i].level, table[i].adr, str); /* 处理字符串赋值 */
					}
				}
			}
		}
	}
	else
	{
		if (sym == scansym)	/* 准备按照read语句处理 */
		{
			getsym();
			if (sym != lparen)
			{
				error(34);	/* 格式错误，应是左括号 */
			}
			else
			{
				do {
					getsym();
					if (sym == ident)
					{
						i = position(id, *ptx);	/* 查找要读的变量 */
					}
					else
					{
						i = 0;
					}

					if (i == 0)
					{
						error(35);	/* read语句括号中的标识符应该是声明过的变量 */
					}
					else
					{
						if (table[i].kind == variable)
						{
							gen(opr, 0, 16);  /* 生成输入指令，读取值到栈顶 */
						}
						else if (table[i].kind == stringvar)
						{
							gen(opr, 0, 17);  /* 生成输入字符串指令 */
						}
						gen(sto, lev - table[i].level, table[i].adr);  /* 将栈顶内容送入变量单元中 */
					}
					getsym();

				} while (sym == comma);	/* 一条read语句可读多个变量 */
			}
			if (sym != rparen)
			{
				error(33);	/* 格式错误，应是右括号 */
				while (!inset(sym, fsys))	/* 出错补救，直到遇到上层函数的后继符号 */
				{
					getsym();
				}
			}
			else
			{
				getsym();
			}
		}
		else if (sym == printsym)    /* 准备按照print语句处理 */
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
						gen(lits, 0, 0, str);  /* 将字符串常量入栈 */
						gen(opr, 0, 21);       /* 生成输出字符串的指令 */
					}
					else
					{
						
						if (table[position(id, *ptx)].kind == stringvar)
						{
							strexpr(nxtlev, ptx, lev);
							gen(opr, 0, 21);   /* 生成输出字符串的指令 */
						}
						else
						{
							expression(nxtlev, ptx, lev);  /* 调用表达式处理 */
							gen(opr, 0, 14);   /* 生成输出整数的指令 */
						}
					}
					gen(opr, 0, 15);           /* 生成换行指令 */
				} while (sym == comma);        /* 一条print可输出多个变量的值 */
				if (sym != rparen)
				{
					error(33);  /* 格式错误，应是右括号 */
				}
				else
				{
					getsym();
				}
			}
		}

		/* 准备按照if语句处理
		if (bool_expr) then {stmt_list} end
		if (bool_expr) then {stmt_list} else {stmt_list} end
		*/
		if (sym == ifsym)  // 处理 if 语句
		{
			getsym();
			if (sym != lparen)
			{
				error(6);  // 缺少左括号
			}
			else
			{
				getsym();
				memcpy(nxtlev, fsys, sizeof(bool) * symnum);
				nxtlev[rparen] = true;
				condition(nxtlev, ptx, lev);  // 处理条件表达式
				if (sym == rparen)
				{
					getsym();
					if (sym == thensym)
					{
						getsym();
						if (sym != lbrace)
						{
							error(7);  // 缺少左花括号
						}
						else
						{
							getsym();
							cx1 = cx;  // 保存当前指令地址
							gen(jpc, 0, 0);  // 生成条件跳转指令，跳转地址未知，暂时写0

							memcpy(nxtlev, fsys, sizeof(bool) * symnum);
							nxtlev[rbrace] = true;
							statementlist(nxtlev, ptx, lev);  // 处理 then 部分的语句列表
							code[cx1].a = cx;  // 回填跳转地址

							if (sym == rbrace)
							{
								getsym();
								if (sym == elsesym)
								{
									getsym();
									if (sym != lbrace)
									{
										error(7);  // 缺少左花括号
									}
									else
									{
										getsym();
										cx2 = cx;  // 保存当前指令地址
										gen(jmp, 0, 0);  // 生成无条件跳转指令，跳转地址未知，暂时写0
										code[cx1].a = cx;  // 回填跳转地址

										memcpy(nxtlev, fsys, sizeof(bool) * symnum);
										nxtlev[rbrace] = true;
										statementlist(nxtlev, ptx, lev);  // 处理 else 部分的语句列表
										code[cx2].a = cx;  // 回填跳转地址

										if (sym == rbrace)
										{
											getsym();
											if (sym != endsym)
											{
												error(6);  // 缺少 end
											}
											else
											{
												getsym();
											}
										}
										else
										{
											error(6);  // 缺少右花括号
										}
									}
								}
								else
								{
									if (sym != endsym)
									{
										error(6);  // 缺少 end
									}
									else
									{
										getsym();
									}
								}
							}
							else
							{
								error(6);  // 缺少右花括号
							}
						}
					}
					else
					{
						error(16);  // 缺少 then
					}
				}
				else
				{
					myerr("缺少)");
				}
			}
		}
		else if (sym == whilesym)	/* 准备按照while语句处理 */
		{
			cx1 = cx;	/* 保存判断条件操作的位置 */
			getsym();
			if (sym != lparen)
			{
				myerr("缺少(");
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
						cx2 = cx; /* 保存循环体的结束的下一个位置 */
						gen(jpc, 0, 0); /* 生成条件跳转，但跳出循环的地址未知，标记为0等待回填 */
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						nxtlev[rbrace] = true;
						statementlist(nxtlev, ptx, lev); /* 处理语句列表 */
						gen(jmp, 0, cx1); /* 生成条件跳转指令，跳转到前面判断条件操作的位置 */
						code[cx2].a = cx; /* 回填跳出循环的地址 */
						if (sym == rbrace)
						{
							getsym();
						}
						else
						{
							error(33); /* 格式错误，应是右花括号 */
						}
					}
					else
					{
						error(34); /* 格式错误，应是左花括号 */
					}
				}
				else
				{
					myerr("缺少)");
				}
			}
		}

	}
	memset(nxtlev, 0, sizeof(bool) * symnum);	/* 语句结束无补救集合 */
	test(fsys, nxtlev, 19);	/* 检测语句结束的正确性 */
}

/*
 * 表达式处理
 */
void expression(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;	/* 用于保存正负号 */
	bool nxtlev[symnum];

	if (sym == plus || sym == minus)	/* 表达式开头有正负号，此时当前表达式被看作一个正的或负的项 */
	{
		addop = sym;	/* 保存开头的正负号 */
		getsym();
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* 处理项 */
		if (addop == minus)
		{
			gen(opr, 0, 1);	/* 如果开头为负号生成取负指令 */
		}
	}
	else	/* 此时表达式被看作项的加减 */
	{
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* 处理项 */
	}
	while (sym == plus || sym == minus)
	{
		addop = sym;
		getsym();
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);	/* 处理项 */
		if (addop == plus)
		{
			gen(opr, 0, 2);	/* 生成加法指令 */
		}
		else
		{
			gen(opr, 0, 3);	/* 生成减法指令 */
		}
	}
}

/*
 * 项处理
 */
void term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;	/* 用于保存乘除法符号 */
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	factor(nxtlev, ptx, lev);	/* 处理因子 */
	while (sym == times || sym == slash)
	{
		mulop = sym;
		getsym();
		factor(nxtlev, ptx, lev);
		if (mulop == times)
		{
			gen(opr, 0, 4);	/* 生成乘法指令 */
		}
		else
		{
			gen(opr, 0, 5);	/* 生成除法指令 */
		}
	}
}

/*
 * 因子处理
 */
void factor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	test(facbegsys, fsys, 24);	/* 检测因子的开始符号 */
	while (inset(sym, facbegsys)) 	/* 循环处理因子 */
	{
		if (sym == ident)	/* 因子为标识符 */
		{
			i = position(id, *ptx);	/* 查找标识符在符号表中的位置 */
			if (i == 0)
			{
				error(11);	/* 标识符未声明 */
			}
			else
			{
				switch (table[i].kind)
				{
				case variable:	/* 标识符为变量 */
					gen(lod, lev - table[i].level, table[i].adr);	/* 找到变量地址并将其值入栈 */
					break;
				default:
					error(21);	/* 标识符类型错误 */
					break;
				}
			}
			getsym();
		}
		else if (sym == number)	/* 因子为数 */
		{
			if (num > amax)
			{
				error(31); /* 数越界 */
				num = 0;
			}
			gen(lit, 0, num);
			getsym();
		}
		else if (sym == lparen)	/* 因子为表达式 */
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
				error(22);	/* 缺少右括号 */
			}
		}
		else
		{
			error(24);	/* 无效的因子 */
		}
		memset(nxtlev, 0, sizeof(bool) * symnum);
		nxtlev[lparen] = true;
		test(fsys, nxtlev, 23); /* 一个因子处理完毕，遇到的单词应在fsys集合中 */
		/* 如果不是，报错并找到下一个因子的开始，使语法分析可以继续运行下去 */
	}
}


//任何独立的，比如（）内的，字符表达式，必须以字符串作为开头
void strexpr(bool* fsys, int* ptx, int lev)
{
	bool nxtlev[symnum];
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[plus] = true;
	strterm(nxtlev, ptx, lev);  // 处理strterm
	while (sym == plus)
	{
		getsym();

		if (sym == ident)
		{
			int i = position(id, *ptx);
			if (i == 0)
			{
				error(11);  // 标识符未声明
			}
			else if (table[i].kind == variable)
			{
				strterm(nxtlev, ptx, lev);
				gen(opr, 0, 20);  // 生成字符串与数字相加指令
			}
			else
			{
				strterm(nxtlev, ptx, lev);  // 处理strterm
				gen(opr, 0, 18);  // 生成字符串相加指令
			}
		}
		else
		{
			strterm(nxtlev, ptx, lev);  // 处理strterm
			gen(opr, 0, 18);  // 生成字符串相加指令
		}
	}
}




void strterm(bool* fsys, int* ptx, int lev)
{
	bool nxtlev[symnum];
	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[times] = true;
	strfactor(nxtlev, ptx, lev);  // 处理strfactor
	while (sym == times)
	{
		getsym();
		
		factor(nxtlev, ptx, lev);  // 处理factor
		gen(opr, 0, 19);  // 生成字符串乘法指令
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
			error(11);  // 标识符未声明
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
				error(21);  // 不能为过程
				break;
			}
		}
		getsym();
	}
	else if (sym == conststr)
	{
		gen(lits, 0, 0, str);  // 生成字符串常量的指令
		getsym();
	}
	else if (sym == number)
	{
		gen(lits, 0, 0,std::to_string(num));  // 生成数字常量的指令
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
			error(22);  // 缺少右括号
		}
	}
	else
	{
		error(24);  // 错误的因子
	}
	memset(nxtlev, 0, sizeof(bool) * symnum);
	nxtlev[lparen] = true;
	test(fsys, nxtlev, 23);  // 一个因子处理完毕，遇到的单词应在fsys集合中
}

/*
 * 条件处理
 */
void condition(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	// L24不支持odd运算符，因此省略odd处理
	
	/* 逻辑表达式处理 */
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
		error(20); /* 应该为关系运算符 */
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
 * 解释程序
 */
void interpret()
{
	int p = 0; /* 指令指针 */
	int b = 1; /* 指令基址 */
	int t = 0; /* 栈顶指针 */
	struct instruction i;	/* 存放当前指令 */
	int s[stacksize];	/* 栈 */
	std::string str_s[stacksize * 3];
	int is = 0;

	printf("Start pl0\n");
	fprintf(fresult, "Start pl0\n");
	s[0] = 0; /* s[0]不用 */
	s[1] = 0; /* 主程序的三个联系单元均置为0 */
	s[2] = 0;
	s[3] = 0;
	do {
		i = code[p];	/* 读当前指令 */
		p = p + 1;
		switch (i.f)
		{
		case lit:	/* 将常量a的值取到栈顶 */
			t = t + 1;
			s[t] = i.a;
			break;
		case lits:		//把instruction.s字符串入str_s栈顶，并把这个这个.s的索引is放到s[]栈顶
			//先把i.s存到str_s里，然后索引is++，把string的索引取到栈顶
			t = t + 1;
			s[t] = is;
			str_s[is] = i.s;
			is++;		//指向下一个位置
			break;
		case opr:	/* 数学、逻辑运算 */
			switch (i.a)
			{
			case 0:  /* 函数调用结束后返回 */
				t = b - 1;
				p = s[t + 3];
				b = s[t + 2];
				break;
			case 1: /* 栈顶元素取反 */
				s[t] = -s[t];
				break;
			case 2: /* 次栈顶项加上栈顶项，退两个栈元素，相加值进栈 */
				t = t - 1;
				s[t] = s[t] + s[t + 1];
				break;
			case 3:/* 次栈顶项减去栈顶项 */
				t = t - 1;
				s[t] = s[t] - s[t + 1];
				break;
			case 4:/* 次栈顶项乘以栈顶项 */
				t = t - 1;
				s[t] = s[t] * s[t + 1];
				break;
			case 5:/* 次栈顶项除以栈顶项 */
				t = t - 1;
				s[t] = s[t] / s[t + 1];
				break;
			case 6:/* 栈顶元素的奇偶判断 */
				s[t] = s[t] % 2;
				break;
			case 8:/* 次栈顶项与栈顶项是否相等 */
				t = t - 1;
				s[t] = (s[t] == s[t + 1]);
				break;
			case 9:/* 次栈顶项与栈顶项是否不等 */
				t = t - 1;
				s[t] = (s[t] != s[t + 1]);
				break;
			case 10:/* 次栈顶项是否小于栈顶项 */
				t = t - 1;
				s[t] = (s[t] < s[t + 1]);
				break;
			case 11:/* 次栈顶项是否大于等于栈顶项 */
				t = t - 1;
				s[t] = (s[t] >= s[t + 1]);
				break;
			case 12:/* 次栈顶项是否大于栈顶项 */
				t = t - 1;
				s[t] = (s[t] > s[t + 1]);
				break;
			case 13: /* 次栈顶项是否小于等于栈顶项 */
				t = t - 1;
				s[t] = (s[t] <= s[t + 1]);
				break;
			case 14:/* 栈顶值输出 */
				printf("%d", s[t]);
				fprintf(fresult, "%d", s[t]);
				t = t - 1;
				break;
			case 15:/* 输出换行符 */
				printf("\n");
				fprintf(fresult, "\n");
				break;
			case 16:/* 读入一个输入置于栈顶 */
				t = t + 1;
				printf("input: ");
				fprintf(fresult, "input: ");
				scanf("%d", &(s[t]));
				fprintf(fresult, "%d\n", s[t]);
				break;
			case 17:	//读入一个字符串置于栈顶
				t = t + 1;
				printf("input: ");
				fprintf(fresult, "input: ");
				std::cin >> str_s[is];
				s[t] = is;			//栈顶存的是这个字符串在str_s中的下标
				is++;
				break;
			case 18:	//字符串+字符串
				t = t - 1;
				str_s[is] = str_s[s[t]] + str_s[s[t + 1]];
				s[t] = is;
				is++;
				break;
			case 19:	//字符串 * n
				t = t - 1;
				str_s[is] = scopy(str_s[s[t]], s[t + 1]);
				s[t] = is;
				is++;
				break;
			case 20:	//字符串+整数num
				t = t - 1;
				str_s[is] = str_s[s[t]] + std::to_string(s[t + 1]);
				s[t] = is;
				is++;
				break;
			case 21:    // 输出字符串
				std::cout << str_s[s[t]] << std::endl;
				t = t - 1;
				break;
			}
			break;
		case lod:	/* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
			t = t + 1;
			s[t] = s[base(i.l, s, b) + i.a];
			break;
		case sto:	/* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
			s[base(i.l, s, b) + i.a] = s[t];
			t = t - 1;
			break;
		case ini:	/* 在数据栈中为被调用的过程开辟a个单元的数据区 */
			t = t + i.a;
			break;
		case jmp:	/* 直接跳转 */
			p = i.a;
			break;
		case jpc:	/* 条件跳转 */
			if (s[t] == 0)
				p = i.a;
			t = t - 1;
			break;
		}
	} while (p != 0);
	printf("End pl0\n");
	fprintf(fresult, "End pl0\n");
}

/* 通过过程基址求上l层过程的基址 */
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

