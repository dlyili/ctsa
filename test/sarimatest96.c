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
	// 方法 0 = MLE (最大似然，内部要先 CSS 估初值再 MLE 精调)
	//     1 = CSS (条件最小二乘，近似估计，速度快很多)
	//     2 = Box-Jenkins (两步法)
	// 对于长季节周期 s=96、季节 AR P=2 的场景，MLE 的 Kalman 滤波状态维数
	// ip = p + s*P = 194，每步 O(N * ip³) ≈ 3.5B FLOP，迭代数百次会很慢
	// 生产环境优先用 CSS 做快速拟合，精度够用且速度可提升 1000× 以上
	sarima_setMethod(obj, 1);
	// 优化器：6 = Limited-Memory BFGS，每步内存开销低，适合大维参数空间；
	// 若遇到收敛告警可换回 5 (BFGS) 或 0 (Nelder-Mead)。
	sarima_setOptMethod(obj, 6);
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
