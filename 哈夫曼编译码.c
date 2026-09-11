
#include<stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {//哈夫曼树节点
	unsigned int weight;  //权值
	unsigned int parent;  //父亲节点
	unsigned int lchild;  //左孩子
	unsigned int rchild;  //右孩子
} HfmNode, * HfmTree;

// 哈夫曼编码表 
typedef char** HfmCode;

void selectMin2(HfmTree HT,int*t1,int*t2,int k) {
	unsigned int min1 = 0xFFFFFFFF, min2 = 0xFFFFFFFF;
	for (int i = 1; i < k; i++) {
		if (HT[i].parent != 0) continue; // 跳过已合并的结点！！！
		if (HT[i].weight < min1) {//先判断最小
			//如果最小更新，那么第二小肯定也要更新
			min2 = min1;
			*t2 = *t1;

			min1 = HT[i].weight;
			*t1 = i;
		}
		else if (HT[i].weight < min2) {//否则判断第二小
			min2 = HT[i].weight;
			*t2 = i;
		}
	}
}


void CreateHfmTree(int* w,HfmTree* HT,int n) {
	int m = 2 * n - 1;//哈夫曼树的节点个数，其中n个为叶子节点即字符，其余n-1个是合并节点

	//hfmTree在函数内malloc（谁生产这份数据，谁负责分配内存）
	*HT = (HfmTree)malloc((m + 1) * sizeof(HfmNode));//0号位置为父节点空判段，不能用，所以+1（0号节点要空出来，0 充当哨兵标记值，代表 “不存在父亲”，所以 0 位置不能存真实树结点。
	
	
	//初始化叶节点（字符）(安排在前n个)（内存上是通过数组直接连接，逻辑上是通过内部父亲孩子变量连接的,所以存储顺序不重要）
	for (int i = 1; i <= n; i++) {
        (*HT)[i].weight = w[i - 1];//注意下标对应
        (*HT)[i].parent = 0;
        (*HT)[i].lchild = 0;
        (*HT)[i].rchild = 0;
    }
    //初始化合并结点（不含字符，属于构造哈夫曼树逻辑上新建的节点）
    for (int i = n + 1; i <= m; i++) {
        (*HT)[i].weight = 0;
        (*HT)[i].parent = 0;
        (*HT)[i].lchild = 0;
        (*HT)[i].rchild = 0;
    }
	//连接哈夫曼树
	for (int i = n+1; i <= m; i++) {//有几个合并节点就是合并连接了几次
		int t1, t2;
		selectMin2(*HT, &t1, &t2,i);//找出1到i（不包含i）之间权值最小(并且没有父节点！！！）的两个节点合并

		//修改各自的父亲与孩子节点完成连接，合并权值
		(*HT)[t1].parent = i;
		(*HT)[t2].parent = i;
		(*HT)[i].lchild = t1;
		(*HT)[i].rchild = t2;
		(*HT)[i].weight = (*HT)[t1].weight + (*HT)[t2].weight;

	}
}

void CreateHfmCode(HfmCode* HC,HfmTree HT,int n) {
	(*HC) = (HfmCode)malloc((n+1) * sizeof(char*));//分配n+1个字符对应的编码字符串(HC的0号也不用只是为了对齐HT而已）

	char* cd = (char*)malloc(n * sizeof(char));//分配临时存放编码空间（n个字符哈夫曼编码最长的为n-1，但是最后要放字符串结束符/0，所以+1）
	cd[n - 1] = '\0';

	for (int i = 1; i <= n; i++) {//遍历给n个字符编码
		//从叶子到根逆向求每个字符的哈夫曼编码

		int start = n - 1;//编码起始位置，一开始在结束符位置，表示未编码

		int cur = i;//当前字符（叶子节点）
		int p = HT[cur].parent;//叶子节点父节点

		while (p != 0) {
			if (HT[p].lchild == cur) cd[--start] = '0';//节点为左孩子编0，起始位置向前--
			else cd[--start] = '1';//节点为右孩子编1，起始位置向前--

			//然后从叶子继续往上直到根没有父亲
			cur = p;
			p = HT[p].parent;
		}
		//最后i位置字符编码长度即为n-start,总空间长度-起始位置下标
		(*HC)[i] = (char*)malloc((n - start) * sizeof(char));
		strcpy((*HC)[i], &cd[start]);//strcpy即从start地址开始，逐个复制字符，直到‘\0’
		//注意cd是char*，但是cd[start]就是char了，所以应该取地址&
	}

	free(cd);//所有字符遍历编码完后记得释放临时cd内存空间
}

void SaveHfmTree(const char* filename,int n,char* chars,HfmTree HT) {
	//文件操作步骤
	//一，先定义文件指针
	FILE* fp;
	//二，打开文件方式
	fp = fopen(filename, "wb");  // wb：write binary 以二进制写方式打开
	//三，判断打开是否成功
	if (fp == NULL) {
		printf("无法创建文件 %s！\n", filename);
		return;
	}

	//四，读写数据
	   // 1.写入叶子结点数量 n 
	fwrite(&n, sizeof(int), 1, fp);
	/*
	fwrite 函数原型

	```
	size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
	```

	参数解释：

	1. `ptr`：**数据源内存地址**，要写什么东西的首地址
	2. `size`：单个元素占多少字节
	3. `nmemb`：一共要写多少个这样的元素
	4. `stream`：文件指针
*/

	   //2.写入n个叶子对应的字符数组 chars
	fwrite(chars, sizeof(char), n, fp);

	   //3.写入哈夫曼树节点（结构体拆开存，防止内存对齐产生问题）（虽然这里不会有内存对齐，但防止未来更改结构体，属于防御性编程）
	for (int i = 1; i <= 2 * n - 1; i++) {
		fwrite(&HT[i].weight, sizeof(unsigned int), 1, fp);
		fwrite(&HT[i].parent, sizeof(unsigned int), 1, fp);
		fwrite(&HT[i].lchild, sizeof(unsigned int), 1, fp);
		fwrite(&HT[i].rchild, sizeof(unsigned int), 1, fp);
	}

	//五，关闭文件
	fclose(fp);
	printf("哈夫曼树已保存到 %s\n", filename);
}

void LoadHfmTree(const char* filename, int* n, char** chars, HfmTree* HT) {
	FILE* fp;
	fp = fopen(filename, "rb");// rb read binary：二进制读模式
	if (fp == NULL) {
		printf("无法打开文件 %s！\n", filename);
		return;
	}
	fread(n, sizeof(int), 1, fp);
	*chars = (char*)malloc((*n) * sizeof(char));//因为内存里没有哈夫曼树，那么chars一定还没有分配内存
	if (!*chars) {
	    printf("内存分配失败！\n");
	    fclose(fp);
	    exit(1);
	}
	fread(*chars, sizeof(char), *n, fp);
	// 分配哈夫曼树内存
	*HT = (HfmTree)malloc((2*(*n)-1 + 1) * sizeof(HfmNode));
	if (!*HT) {
		printf("内存分配失败！\n");
		free(*chars);
		fclose(fp);
		exit(1);
	}
	for (int i = 1; i <= 2 * (*n) - 1; i++) {
		fread(&(*HT)[i].weight, sizeof(unsigned int), 1, fp);
		fread(&(*HT)[i].parent, sizeof(unsigned int), 1, fp);
		fread(&(*HT)[i].lchild, sizeof(unsigned int), 1, fp);
		fread(&(*HT)[i].rchild, sizeof(unsigned int), 1, fp);
	}
	fclose(fp);
	printf("哈夫曼树已从 %s 加载\n", filename);
}

void EncodeFile(const char* inputFile,const char* outputFile,int n,char* chars,HfmCode HC) {
		FILE* fin, *fout;
		fin = fopen(inputFile, "r");
		if (!fin) {
			printf("无法打开文件 %s！\n", inputFile);
			return;
		}
		fout = fopen(outputFile, "w");
		if (!fout) {
			printf("无法创建文件 %s！\n", outputFile);
			fclose(fin);//记得关闭前面成功打开的fin再return
			return;
		}

		int ch;
		/*

		```
		int fgetc(FILE *fp);
		```

		> 
		> 从文件读取**一个字符**。返回类型是`int`！这点非常关键！

		- 读取到普通字符：返回该字符的 ASCII 码；
		- **读到文件末尾：返回宏 `EOF`（一般是 - 1）。

		> 
		> 为什么返回 int？因为 char 范围 0~255，EOF=-1 不在 char 范围内。如果存到 char 变量，会无法区分正常字符和文件结束标记。
		> 所以代码用 `int ch;` 接收返回值。
		*/
		while ((ch = fgetc(fin)) != EOF) {//一个一个字符遍历读
			//遍历查找字符对应的编码
			int i;
			for (i = 0; i < n; i++) {
				if (chars[i] == ch) {
					fputs(HC[i + 1], fout);//注意下标对应，一个一个找到对应编码之后写入fout
					//`fputs` 输出的是 `'\0'` 结尾的字符串,"fputc"输出单个字符
					break;
				}
			}
			if (i == n) {//如果上面没找到对应编码（没有break）说明有未编码字符
				printf("警告：文件中存在未定义字符 '%c'，已忽略\n", ch);
			}
		}
		fclose(fin);
		fclose(fout);
		printf("编码完成，结果已保存到 %s\n", outputFile);
}

void DecodeFile(const char* inputFile, const char* outputFile ,int n,HfmTree HT,char* chars) {
	FILE* fin, * fout;
	fin = fopen(inputFile, "r");
	if (!fin) {
		printf("无法打开文件 %s！\n 请先编码\n", inputFile);
		return;
	}
	fout = fopen(outputFile, "w");
	if (!fout) {
		printf("无法创建文件 %s！\n", outputFile);
		fclose(fin);//记得关闭前面成功打开的fin再return
		return;
	}
	int ch;
	int p = 2 * n - 1;//从根节点开始译码
	while ((ch = fgetc(fin)) != EOF) {
		if (ch == '0') {//顺着01编码往叶节点找
			p = HT[p].lchild;
		}
		else if (ch == '1') {
			p = HT[p].rchild;
		}
		else {//非01字符跳过
			continue;
		}
		if (HT[p].lchild == 0 && HT[p].rchild == 0) {//找到叶节点
			fputc(chars[p - 1], fout);//写入该叶节点对应字符
			//`fputs` 输出的是 `'\0'` 结尾的字符串,"fputc"输出单个字符
			p = 2 * n - 1;//回到根节点继续
		}
	}
	fclose(fin);
	fclose(fout);
	printf("译码完成，结果已保存到 %s\n", outputFile);
}

void PrintCodeFile(const char* filename) {
	    FILE* fp;
	    int ch;
	    int count = 0;
	    fp = fopen(filename, "r");
	    if (!fp) {
	        printf("无法打开文件 %s！\n", filename);
	        return;
	    }
	    printf("编码文件内容（每行50个）：\n");
	    while ((ch = fgetc(fp)) != EOF) {
	        if (ch == '0' || ch == '1') {
	            putchar(ch);
	            count++;
	            if (count % 50 == 0) {
	                printf("\n");
	            }
	        }
	    }
	    if (count % 50 != 0) {
	        printf("\n");
	    }
	    fclose(fp);
}

void PrintHfmTreeRec(HfmTree HT,FILE* fp,int dep,int node,char* chars) {
	//凹入表根据深度打印空格缩进
	for (int i = 0; i < dep; i++) {
		fprintf(fp, "	");
		printf("	");
	}

	/* 叶子结点：输出字符和权值 */
    if (HT[node].lchild == 0 && HT[node].rchild == 0) {
        fprintf(fp, "└─ %c (权值: %u)\n", chars[node - 1], HT[node].weight);
        printf("└─ %c (权值: %u)\n", chars[node - 1], HT[node].weight);
    }
    else {
        /* 合并结点：输出权值 */
        fprintf(fp, "├─ 合并权值: %u\n", HT[node].weight);
        printf("├─ 合并权值: %u\n", HT[node].weight);
        /* 递归左子树和右子树 */
        PrintHfmTreeRec(HT,fp, dep+1,HT[node].lchild, chars);
        PrintHfmTreeRec(HT,fp,dep+1, HT[node].rchild, chars);
    }
}

void PrintHfmTree(HfmTree HT,const char* filename,int n,char* chars) {
	FILE* fp;
	fp = fopen(filename, "w");
	if (!fp) {
		printf("无法创建文件 %s！\n", filename);
		return;
	}
	//打印到控制台的同时写入文件
	printf("哈夫曼树结构：\n");
	fprintf(fp, "哈夫曼树结构（凹入表形式）：\n");
	//递归打印和写入
	PrintHfmTreeRec(HT, fp,0, 2*n-1, chars);
	fclose(fp);
	printf("哈夫曼树已保存到 %s\n", filename);
}

// 释放所有内存
void FreeAll(HfmTree* HT, HfmCode* HC, char** chars, int n) {
	if (*HT) {//HT是一维结构体数组，一次 malloc，一次 free.  HT 是一块连续、整块的内存，存放一堆结构体。
		free(*HT);
		*HT = NULL;
	}
	/*而HC 是指针数组（二级指针），分两层内存
	// HC 是 char** ;
	HC = (char**)malloc( (n+1)*sizeof(char*) );

	//然后每个编码字符串单独 malloc
	HC[i] = (char*)malloc(长度 * sizeof(char));
	```

	内存结构：

	- **第一层**：HC 本身，一块数组，里面存一堆指针 `HC[1], HC[2], HC[3]…`
	- **第二层**：每一个 `HC[i]` 单独指向一块堆内存，存 `"0110"` 这种字符串*/
	if (*HC) {
		for (int i = 1; i <= n; i++) {//所以要遍历释放第二层
			free((*HC)[i]);
		}
		free(*HC);//最后释放第一层
		*HC = NULL;
	}
	if (*chars) {
		free(*chars);
		*chars = NULL;
	}
}

int main() {
	int n = 0;//字符集大小
	char* chars = NULL;//字符集字符数组指针
	int* w = NULL;//字符集字符对应权值数组指针
	HfmTree hfmTree = NULL;//哈夫曼节点数组指针
	HfmCode hfmCode = NULL;//哈夫曼编码数组指针（指向char*（char*即c语言字符串））（只存编码后的字符串，编码前的原字符之间通过下标对应在chars里面找）
    //以上数组均通过下标对应产生联系

	while (1) {
		char cmd;
		printf("\n功能菜单：\n");
		printf("I - 初始化（建立哈夫曼树）\n");
		printf("E - 编码\n");
		printf("D - 译码\n");
		printf("P - 打印代码文件\n");
		printf("T - 打印哈夫曼树\n");
		printf("Q - 退出\n");
		printf("请输入指令：");
		scanf(" %c", &cmd);//空格跳过空白字符 (/n之类的)
		switch (cmd) {
			//初始化
		case 'I': case'i': {
			// 先释放旧内存,防止多次执行I命令会内存泄漏
			FreeAll(&hfmTree, &hfmCode, &chars, n);
			if (w) { free(w); w = NULL; }

			printf("请输入字符集大小n：");
			scanf("%d", &n);
			while (getchar() != '\n'); // 吃掉n输入后的残留换行
			if (n <= 1) {
				printf("字符集大小必须大于1！\n");
				// 释放已分配内存，回到菜单
				break;
			}


			//动态分配数组大小
			chars = (char*)malloc(n * sizeof(char));
			w = (int*)malloc(n * sizeof(int));

			//输入字符数据
			printf("请依次输入%d个字符及其权值：\n", n);
			for (int i = 0; i < n; i++) {
			    printf("第%d个字符：", i + 1);
			    chars[i] = getchar();//字符可能本身就是空格，所以不能用空格跳过，而且得用getchar（scanf("%c", &chars[i]);不加空格，%c会原样读取任何字符，包括换行符'/n')
			    while (getchar() != '\n');//把本行后面所有剩余字符，直到换行全部丢弃
			    printf("对应权值：");
			    scanf("%d", &w[i]);
			    while (getchar() != '\n');
			}
			//构建哈夫曼树
			CreateHfmTree(w,&hfmTree,n);//逻辑上构建哈夫曼树时，用不到chars，只看权值，后续通过下标找chars字符即可
			//注意这里传入的是指向HfmTree的指针，即HfmNode指针的指针，因为这样才是引用传递，才能修改，直接传HfmTree是值传递，不会影响原参数
			
			//同时顺带构建好哈夫曼编码表
			CreateHfmCode(&hfmCode,hfmTree,n);//同理传入的是指向HfmCode的指针,这里就不用传指向HfmTree的指针了，因为只需要读取

			//保存到文件 
			SaveHfmTree("hfmTree", n, chars, hfmTree);

			//释放w
			free(w);
			w = NULL;
			printf("初始化完成！\n");
			break;
		}

			//编码
		case 'E': case'e': {
			//先看内存里有没有哈夫曼树
			if (hfmTree == NULL) {//没有就从文件里读取
				LoadHfmTree("hfmTree", &n, &chars, &hfmTree);
				if (hfmTree == NULL) {//如果文件也没有
					printf("请先执行初始化（I）！\n");
					break;
				}
				//生成哈夫曼编码表
				CreateHfmCode(&hfmCode, hfmTree, n);
			}
			//内存已经有了或者从文件里读好了就继续
			EncodeFile("ToBeTran.txt", "CodeFile.txt", n, chars, hfmCode);

			break;
		}

			//译码
		case 'D' : case'd':
			//先看内存里有没有哈夫曼树
			if (hfmTree == NULL) {//没有就从文件里读取
				LoadHfmTree("hfmTree", &n, &chars, &hfmTree);
				if (hfmTree == NULL) {//如果文件也没有
					printf("请先执行初始化（I）！\n");
					break;
				}
				//生成哈夫曼编码表
				CreateHfmCode(&hfmCode, hfmTree, n);
			}
			DecodeFile("CodeFile.txt", "TextFile.txt", n, hfmTree, chars);
			break;

			//打印代码文件
		case 'P' : case'p':
			PrintCodeFile("CodeFile.txt");
			break;

			//打印哈夫曼树并保存文件
		case 'T' : case 't':
			//先看内存里有没有哈夫曼树
			if (hfmTree == NULL) {//没有就从文件里读取
				LoadHfmTree("hfmTree", &n, &chars, &hfmTree);
				if (hfmTree == NULL) {//如果文件也没有
					printf("请先执行初始化（I）！\n");
					break;
				}
				//生成哈夫曼编码表
				CreateHfmCode(&hfmCode, hfmTree, n);
			}
			//打印
			PrintHfmTree(hfmTree,"TreePrint.txt",n,chars);
			break;
		case 'Q' : case'q': {
			/* 退出，释放内存 */
			FreeAll(&hfmTree, &hfmCode, &chars, n);
			printf("程序已退出。\n");
			system("pause");
			return 0;
		}
		default:
			printf("输入错误，请重新选择！\n");
			break;
		}
	}
}