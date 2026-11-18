// SPDX-License-Identifier: BSD-3-Clause
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../header/ctsa.h"

int main(void) 
{
	int i, N, L;
	double* inp;
	int p, d, q;
	int P, D, Q, s;
	double* xpred, * amse;
	sarima_object obj;

	//时间序列长度,一天共计48个时刻(每半小时一个数据)
	s = 96;
	FILE* ifp = fopen("../data/sarima_96.txt", "r");
	FILE* ofp = fopen("../data/sarima_predict_96.txt", "w");

	//order(p,d,q) deepseek给出的解释
	//p 使用过去多少个时间点的值来预测当前值 AR参数
	//d 为了使序列平稳而进行的差分次数,值=1进行一阶差分,值=2进行二阶差分
	//q 使用过去多少个时间点的预测误差来改进当前预测
	p = 2;	//今天的值与昨天,前天的值有关
	d = 0;	//序列本身平稳,不对序列进行差分
	q = 1;	//今天的预测考虑昨天的预测误差
	//季节性seasonal_order(P,D,Q)
	P = 2;
	D = 1;
	Q = 0;
	L = s;

	xpred = (double*)malloc(sizeof(double) * L);
	amse = (double*)malloc(sizeof(double) * L);
	double temp[1200];

	i = 0;
	if (!ifp)
	{
		printf("Cannot Open File");
		exit(100);
	}
	while (!feof(ifp))
	{
		fscanf(ifp, "%lf \n", &temp[i]);
		i++;
	}
	N = i;

	inp = (double*)malloc(sizeof(double) * N);
	//wmean = mean(temp, N);

	for (i = 0; i < N; ++i)
	{
		inp[i] = temp[i];
	}

	obj = sarima_init(p, d, q, s, P, D, Q, N);
	sarima_setMethod(obj, 0); // Method 0 ("MLE") is default so this step is unnecessary. The method also accepts values 1 ("CSS") and 2 ("Box-Jenkins")
	//sarima_setOptMethod(obj, 7);// Method 7 ("BFGS with More Thuente Line Search") is default so this step is unnecessary. The method also accepts values 0,1,2,3,4,5,6. Check the documentation for details.
	sarima_exec(obj, inp);
	sarima_summary(obj);
	// Predict the next 5 values using the obtained ARIMA model
	sarima_predict(obj, inp, L, xpred, amse);
	printf("\n");
	printf("Predicted Values : \n");
	for (i = 0; i < L; ++i)
	{
		printf("%d	%g \n", i, xpred[i]);
		fprintf(ofp, "%lf \n", xpred[i]);
	}
	printf("\n");
	printf("Standard Errors  : ");
	for (i = 0; i < L; ++i)
	{
		//printf("%g ", sqrt(amse[i]));
	}
	printf("\n");
	sarima_free(obj);
	free(inp);
	free(xpred);
	free(amse);

	fclose(ifp);
	fclose(ofp);

	return 0;

}
