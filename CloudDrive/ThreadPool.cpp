//#include "GlobalVar.h"
//#include "ThreadPool.h"

///*********************************************************
//Create and Setup the threadpool及相关对象
//	this thread pool is a global thread pool.
//	uploading/downloading files in background use this thread pool,
//	but other operation use a seperate threa!!
//	the Maxium thread in this thread pool is 1 .
//*********************************************************/
//VOID SetupThreadPool( void )
//{
//	//Create the thread pool  and set the minimum and maxmum 
//	g_pThreadPool = CreateThreadpool(NULL);
//	if (!g_pThreadPool)
//	{
//		return ;
//	}
//	
//	SetThreadpoolThreadMinimum(g_pThreadPool, 1);
//	SetThreadpoolThreadMaximum(g_pThreadPool, 1);
//
//	InitializeThreadpoolEnvironment(&g_CBEnviron);
//	
//	SetThreadpoolCallbackPool(&g_CBEnviron, g_pThreadPool);
//
//	g_pCleanupGroup = CreateThreadpoolCleanupGroup();
//	SetThreadpoolCallbackCleanupGroup(&g_CBEnviron, g_pCleanupGroup, NULL);  //这个究竟需不需要呢???
//}
//
//
///**************************************************************
//和SetupThreadpool()相对应, 用于关闭并释放相关对象
//*************************************************************/
//VOID CleanupThreadpool()
//{
//	if (g_pCleanupGroup != NULL)
//	{	//release members of the cleanup group
//		CloseThreadpoolCleanupGroupMembers(g_pCleanupGroup, TRUE,   NULL);
//		// Close the cleanup group.
//		CloseThreadpoolCleanupGroup(g_pCleanupGroup);
//	}
//
//	// if a thread pool was created	 close it.
//	if (g_pThreadPool != NULL)
//	{
//		CloseThreadpool(g_pThreadPool);
//	}
//
//	// Destroy the thread pool environment.
//	DestroyThreadpoolEnvironment(&g_CBEnviron);
//}
