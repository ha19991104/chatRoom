#include "db.h"
#include <muduo/base/Logging.h>

// 数据库配置信息  先使用默认，后面是需要从配置文件里面读出来的
static string server = "127.0.0.1";
static string user = "root";
static string password = "root321";
static string dbname = "chatRoom";
/*
typedef struct st_mysql
{
  NET		net;			
  unsigned char	*connector_fd;	
  char		*host,*user,*passwd,*unix_socket,*server_version,*host_info;
  char          *info, *db;
  struct charset_info_st *charset;
  MYSQL_FIELD	*fields;
  MEM_ROOT	field_alloc;
  my_ulonglong affected_rows;
  my_ulonglong insert_id;	
  my_ulonglong extra_info;	
  unsigned long thread_id;		
  unsigned long packet_length;
  unsigned int	port;
  unsigned long client_flag,server_capabilities;
  unsigned int	protocol_version;
  unsigned int	field_count;
  unsigned int 	server_status;
  unsigned int  server_language;
  unsigned int	warning_count;
  struct st_mysql_options options;
  enum mysql_status status;
  my_bool	free_me;		
  my_bool	reconnect;	
  char	        scramble[SCRAMBLE_LENGTH+1];
  my_bool unused1;
  void *unused2, *unused3, *unused4, *unused5;
  LIST  *stmts; 
  const struct st_mysql_methods *methods;
  void *thd;
  my_bool *unbuffered_fetch_owner;
  char *info_buffer;
  void *extension;
} MYSQL;


*/

// 初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr); //创建一个指向mysql的对象？
    /*
    MYSQL *mysql_init(MYSQL *mysql); 内存不足，错误返回NULL。
    入参指定为空，就创建一个MYSQL对象并初始化，然后返回初始的对象指针
    */
}
// 释放数据库连接资源 
MySQL::~MySQL()
{
    if (_conn != nullptr)
        mysql_close(_conn);  // 关闭释放连接
    /*
        void STDCALL mysql_close(MYSQL *sock); 关闭mysql_init分配的对象
    */
}
// 连接数据库
bool MySQL::connect()  //创建连接
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                    password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    /*
        mysql_real_connect() 连接数据库，成功返回MYSQL*连接句柄，失败返回NULL
        如果连接成功，p与_conn的值相同
        MYSQL *		STDCALL mysql_real_connect(
                       MYSQL *mysql,  //mysql_init()返回的值
                       const char *host,// mysql所在的主机ip
					   const char *user,// mysql的用户名
					   const char *passwd,// 密码
					   const char *db,  // 要使用的数据库名字
					   unsigned int port, //mysql的端口号，填0，默认是3306 
					   const char *unix_socket, //本地套接字？ 不使用指定为null
					   unsigned long clientflag); // 通常指定为0
    */
    if (p != nullptr)
    {
        // C和C++代码默认的编码字符是ASCII，如果不设置gbk，从mysql上拉下来的中文显示会乱码
        mysql_query(_conn, "set names gbk");
        /*
       int		STDCALL mysql_query(MYSQL *mysql, const char *q); 查询数据库某个表的内存
       查询成功返回0，失败返回非0值
        */
        LOG_INFO << "connect mysql success!";
    }
    else 
    {
        LOG_INFO << "connect mysql fail!";
    }
    return p;
}
// 更新操作    insert  delete等操作
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                    << sql << "更新失败!";
        return false;
    }
    return true;
    /*
    int	mysql_query(MYSQL *mysql, const char *q); 

    */
}
// 查询操作   select
MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                    << sql << "查询失败!";
        return nullptr;
    /*
     int mysql_query(MYSQL *mysql, const char *q);  mysql是连接成功的指针句柄，q是一个可以执行的sql语句，包括增删改查，结尾不需要加';'
     //查询成功返回0，失败返回非0值
    */
    }
    /*TIP: 查询离线消息的时候，为什么mysql_use_result有多条消息，而不是一条*/
    return mysql_use_result(_conn); //通过句柄指针_conn获得 MYSQL_RES指针，里面包含结果集，出现错误或者没找到返回NULL
   
    /*
    typedef char **MYSQL_ROW; return data as array of strings   二级指针，可以看成是一级指针的数组，每个元素都是一个一级指针，而一级指针又是不同元素的字符串结果

    mysql_use_result 查询的结果只会保存一行在客户端，分配一个MYSQL_RES结构体，结果保存在结构体中
     mysql_store_result 会将查询的结果全部保存到客户端，分配一个MYSQL_RES结构体，结果全部保存在结构体中
    */   
}

//获取连接
MYSQL* MySQL::getConnection()
{
    return _conn;
}


/*
typedef struct st_mysql_res {
  my_ulonglong  row_count;
  MYSQL_FIELD	*fields;
  MYSQL_DATA	*data;
  MYSQL_ROWS	*data_cursor;
  unsigned long *lengths;		
  MYSQL		*handle;
  const struct st_mysql_methods *methods;
  MYSQL_ROW	row;	
  MYSQL_ROW	current_row;	
  MEM_ROOT	field_alloc;
  unsigned int	field_count, current_field;
  my_bool	eof;			
  my_bool       unbuffered_fetch_cancelled;  
  void *extension;
} MYSQL_RES;

*/
