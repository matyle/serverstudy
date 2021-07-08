#include "http_conn.h"
#include <map>
#include "../epoll/Epoll.h"
//定义http响应的状态信息
#include <fstream>
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your Request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file from this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The request file was notfound on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file\n";

const char* doc_root = "var/www/html";


int http_conn::m_user_count = 0;
//初始化数据库链接
map<string, string> users;
void http_conn::initmysql(SqlPool* sqlPool){
    MYSQL *mysql = NULL;
    ConnectionRAII myconn(mysql,sqlPool);//取一个链接从连接池 传了拷贝sqlpool指针对象

    //
    if(mysql_query(mysql,"SELECT username password FROM user")){
        //LOG_ERROR("SELECT ERROR:%s\n",mysql_error(mysql));
    }
    MYSQL_RES * result = mysql_store_result(mysql);// //从表中检索完整的结果集

    int numFields = mysql_num_fields(result); //传地址类型 //返回结果集中的列数

    MYSQL_FIELD *field = mysql_fetch_field(result);//返回所有字段结构的数组

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while(MYSQL_ROW row = mysql_fetch_row(result)){
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;
    } 

}
void http_conn::close_conn(bool real_close){ //关闭一个客户端链接
    if(real_close&&(m_sockfd!=-1)){ //已链接
        //epoll_.remove_fd(m_epollfd,m_sockfd);
        m_user_count--; //用户数量减1
        printf("close one connect\n");
        //m_sockfd = -1;
    }
}
void http_conn::init(int sockfd,const sockaddr_in &addr,char* root,int TRIGMode,int close_log, string user, string passwd, string sqlname){
    m_sockfd = sockfd;
    m_address = addr;
    //调试 避免TIME_WAIT状态
    int reuse = 1;
    //setsockopt(m_sockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    //epoll_.add_fd(m_sockfd,0,0); 
    //addfd(m_epollfd, sockfd, true, m_TRIGMode);

    doc_root = root;
    m_TRIGMode = TRIGMode;
    m_close_log = close_log;

    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name, sqlname.c_str());

    init();

    m_user_count++;
    init();
}

void http_conn::init(){
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_content_length = 0;
    m_host = 0;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    cgi = 0;
    mysql_ = NULL;
    memset(m_read_buf,'\0',READ_BUFFER_SIZE);
    memset(m_write_buf,'\0',WRITE_BUFFER_SIZE);
    memset(m_real_file,'\0',FILENAME_LEN);
}

//循环读取客户数据 直到无数据可读或者对方关闭连接
bool http_conn::read_once(){
    if(m_read_idx>=READ_BUFFER_SIZE){ //读缓冲区满了
        return false;
    }
    int bytes_read = 0;
    while(true){
        bytes_read = recv(m_sockfd,m_read_buf+m_read_idx,READ_BUFFER_SIZE-m_read_idx,0);//从m_read_idx开始写入内存buf，写完之后记得更新idx地址
        if(bytes_read==-1){
            /*
            *返回 值 <0时并且(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)的情况 下认为连接是正常的，
            *继续接收。只是阻塞模式下recv会阻塞着接收数据，
            *非阻塞模式下如果没有数据会返回，不会阻塞着读，因此需要 循环读取
            */
            if(errno==EAGAIN||EWOULDBLOCK) 
                break; //这两种错误不停止认为连接正常 继续循环 /非阻塞ET模式下，需要一次性将数据读完 
            return false;
        }
        else if(bytes_read==0) return false; //结束读取

        m_read_idx += bytes_read; //更新 m_read_idx
    }
    return true;
}


//主状态机 封装
http_conn::HTTP_CODE http_conn::process_read(){
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text = 0;  
    //读取buffer中所有完整的行
    // 要么是检查POST的消息体，在POST请求报文中，消息体的末尾没有任何字符，所以不能使用从状态机的状态，这里转而使用主状态机的状态作为循环入口条件。
    //line_status==LINE_OK 解析完消息体后，报文的完整解析就完成了，但此时主状态机的状态还是CHECK_STATE_CONTENT， 符合循环入口条件，还会再次进入循环，这并不是我们所希望的。
    // 要么是利用parse_line \r\n不能使用从状态机的状态
    
    while(((m_check_state == CHECK_STATE_CONTENT)&&(line_status==LINE_OK))||((line_status=parse_line())==LINE_OK)){ //根据从状态机转移

        text = get_line(); //text为一行
        m_start_line = m_checked_idx; //把当前检查的号赋给行号
        printf("got 1 line : %s \n",text); 
        //三种状态转移逻辑
        //解析http请求行，获取请求方法，目标url，版本号
        /*
        CHECK_STATE_REQUESTLINE，解析请求行

        CHECK_STATE_HEADER，解析请求头

        CHECK_STATE_CONTENT，解析消息体，仅用于解析POST请求
        */
        switch(m_check_state){
            case CHECK_STATE_REQUESTLINE:
            {
                ret = parse_requset_line(text); //调用
                if(ret == BAD_REQUEST){
                    return BAD_REQUEST;
                }
                break;
            }
            case CHECK_STATE_HEADER:
            {
                ret = parse_headers(text);
                if(ret == BAD_REQUEST){
                    return BAD_REQUEST;
                }
                else if(ret == GET_REQUEST){ //如果是GET请求
                    return do_request();
                }
                break;
            }
            //解析消息体
            case CHECK_STATE_CONTENT:
            {
                ret = parse_content(text);
                //完整解析Post请求后
                if(ret == GET_REQUEST){
                    return do_request();//跳转到报文响应函数
                }
                line_status = LINE_OPEN; //完成报文解析，避免再次进入循环 更新line_status 前面循环 需要&&(line_status==LINE_OK
                break;

            }
            default:
                return INTERNAL_ERROR;


        }
    }
    return NO_REQUEST;
}

//从状态机 
//解析一行，解析一行
/*
LINE_OK，完整读取一行

LINE_BAD，报文语法有误

LINE_OPEN，读取的行不完整
*/
http_conn::LINE_STATUS http_conn::parse_line(){
    char temp;
    for(;m_checked_idx<m_read_idx;++m_checked_idx){
        temp = m_read_buf[m_checked_idx]; //
        //判断当前字符是否是\r
        if(temp == '\r'){
            //到达缓存区已读字节末尾，说明缓存区还没收到一行 还需要继续接收。然后会继续接收
            if((m_checked_idx+1)==m_read_idx){
                return LINE_OPEN;
            }
            //如果遇到了
            else if(m_read_buf[m_checked_idx+1]=='\n'){
                //位置设置为\0表示为一行
                //从状态机已经将每一行的末尾\r\n符号改为\0\0，以便于主状态机直接取出对应字符串进行处理。
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;

            }
            return LINE_BAD;
        }
        //如果当前字符是\n,m_checked_idx至少是缓存的第2个，且前面一个是\r
        else if (temp = '\n'){
            if(m_checked_idx>1 && m_read_buf[m_checked_idx-1] == '\r'){ 
                m_read_buf[m_checked_idx-1] = '\0';//必须要/n结尾
                m_read_buf[m_checked_idx+1] = '\0';//
                return LINE_OK;
            }
            return LINE_BAD;
        }
        //没有找到\r\n
        return LINE_OPEN;

    }
}


http_conn::HTTP_CODE http_conn::parse_requset_line(char* text){
    m_url = strpbrk(text," \t");//查看是否有空格或者制表符
    if(!m_url) return BAD_REQUEST;
    *m_url++ = '\0';

    char* method = text; //text方法
    if(strcasecmp(method,"GET")==0){
        m_method = GET;
    }
    else if(strcasecmp(method,"POST")==0){
        m_method = POST;
        cgi = 1;
    }
    else return BAD_REQUEST;

    m_url += strspn(m_url," \t"); //跳过当前空格或者|t 检索字符串 str1 中第一个不在字符串 str2 中出现的字符下标。
    m_version = strpbrk(m_url," \t"); //第二个空格
    if(!m_version) return BAD_REQUEST;

    *m_version ++ = '\0';
    m_version += strspn(m_version," \t");
    if(strcasecmp(m_version,"HTTP/1.1")!=0){
        return BAD_REQUEST;
    }
    if(strncasecmp(m_url,"http://",7) == 0){ //匹配前n个字符串 
        m_url += 7;
        m_url = strchr(m_url,'/'); //参数 str 所指向的字符串中搜索第一次出现字符 c（一个无符号字符）的位置。
    }
    if(!m_url||m_url[0]!='/'){
        return BAD_REQUEST;
    }
    m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

//解析http请求头部信息
http_conn::HTTP_CODE http_conn::parse_headers(char* text){
    if(text[0]=='\0'){
        //遇到空行解析完毕
        if(m_content_length!=0){  //Post请求
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    //connection头部字段
    else if(strncasecmp(text,"Connection:",11)==0){
        text += 11;
        text += strspn(text," \t");//第一个不在 “ \t” 中的位置
        if(strcasecmp(text,"keep-alive")==0){
            m_linger = true;//保持连接
        }
    }

    //处理contentlength头部字段
    else if(strncasecmp(text,"Content-length:",15)==0){
        text += 15;
        text += strspn(text," \t");
        m_content_length = atol(text);
    }
    //处理host头部字段
    else if(strncasecmp(text,"host:",5)==0){
        text+= 5;
        text += strspn(text," \t");
        m_host = text;
    }
    else{
        printf("unknow header %s\n",text);
    }
    return NO_REQUEST;
}


// 没有真正解析消息 只是判断是否完整读入
http_conn::HTTP_CODE http_conn::parse_content(char* text){
    if(m_read_idx>=(m_content_length+m_checked_idx)){
        text[m_content_length] = '\0';
        m_string = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}




/*
HTTP响应 部分。后期需要拆开来 太多太乱了。。
*/

//得到一个正确的http请求时，分析目标文件的属性，如果目标文件存在，且对用户可读，而且不是目录，则使用mmap将其映射到m_file_address，并告诉调用者获取成功

const char* doc_root="/home/qgy/github/ini_tinywebserver/root";

http_conn::HTTP_CODE http_conn::do_request(){
    strcpy(m_real_file,doc_root);
    int len = strlen(doc_root);

    
    const char *p = strrchr(m_url, '/');  //找到/符号的位置
    if(cgi==1&&*(p+1)=='2'||*(p+1)=='3'){
        //根据标志判断是登录检测还是注册检测
       //同步线程登录校验
       //CGI多进程登录校验
               char flag = m_url[1];

        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/");
        strcat(m_url_real, m_url + 2);
        strncpy(m_real_file + len, m_url_real, FILENAME_LEN - len - 1);
        free(m_url_real);

        //将用户名和密码提取出来
        //user=123&passwd=123
        char name[100], password[100];
        int i;
        for (i = 5; m_string[i] != '&'; ++i)
            name[i - 5] = m_string[i];
        name[i - 5] = '\0';

        int j = 0;
        for (i = i + 10; m_string[i] != '\0'; ++i, ++j)
            password[j] = m_string[i];
        password[j] = '\0';

        if (*(p + 1) == '3')
        {
            //如果是注册，先检测数据库中是否有重名的
            //没有重名的，进行增加数据
            char *sql_insert = (char *)malloc(sizeof(char) * 200);
            strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
            strcat(sql_insert, "'");
            strcat(sql_insert, name);
            strcat(sql_insert, "', '");
            strcat(sql_insert, password);
            strcat(sql_insert, "')");

            if (users.find(name) == users.end())
            {   
                int res;
                {
                MutexLockGuard locker(mutex_);
                res = mysql_query(mysql_, sql_insert);
                users.insert(pair<string, string>(name, password));
                }

                if (!res)
                    strcpy(m_url, "/log.html");
                else
                    strcpy(m_url, "/registerError.html");
            }
            else
                strcpy(m_url, "/registerError.html");
        }
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        else if (*(p + 1) == '2')
        {
            if (users.find(name) != users.end() && users[name] == password)
                strcpy(m_url, "/welcome.html");
            else
                strcpy(m_url, "/logError.html");
        }
    }
    if(*(p+1)=='0'){
        char* m_url_real = (char*)malloc(sizeof(char)*200);
        strcpy(m_url_real,"/register.html");
        strncpy(m_real_file+len,m_url_real,strlen(m_url_real));//
        free(m_url_real);

    }

    if(*(p+1)=='1'){
        char* m_url_real = (char*)malloc(sizeof(char)*200);
        strcpy(m_url_real,"/log.html");
        strncpy(m_real_file+len,m_url_real,strlen(m_url_real));//
        free(m_url_real);

    }
    else{
        //均不符合 即不是登录和注册，直接将url与网站目录拼接
        //这里的情况是welcome界面，请求服务器上的一个图片
        strncpy(m_real_file+len,m_url,FILENAME_LEN-len-1); 
    }
   


    if(stat(m_real_file,&m_file_stat)<0) return NO_RESOURE; //文件为空
    if(!(m_file_stat.st_mode&S_IROTH)) return FROBIDDEN_REQUEST; //权限不足
    if(S_ISDIR(m_file_stat.st_mode)) return BAD_REQUEST; //请求一个文件夹
    int fd = open(m_real_file,O_RDONLY);
    m_file_address = (char*) mmap(0,m_file_stat.st_size,PROT_READ,MAP_PRIVATE,fd,0); //第一个参数为0 由系统指派映射地址

    close(fd);

    //表示文件存在可以访问
    return FILE_REQUEST;
}

bool http_conn::process_write(HTTP_CODE ret){
    //根据服务器处理http请求的结果决定返回给客户端的内容
    switch(ret){
        //内部错误，500
        case INTERNAL_ERROR:
        {
            add_status_line(500,error_500_title);
            add_headers(strlen(error_500_form));
            if(!add_content(error_500_form)){
                return false;
            }
            break;
        }
        //报文语法有误，404
        case BAD_REQUEST:
        {
            add_status_line(400,error_400_title);
            add_headers(strlen(error_400_form));
            if(!add_content(error_400_form)){
                return false;
            }
            break;
        }
        //资源没有访问权限，403
        case NO_RESOURE:
        {
            add_status_line(403,error_403_title);
            add_headers(strlen(error_403_form));
            if(!add_content(error_403_form)){
                return false;
            }
            break;
        }
        //文件存在，200
        case FILE_REQUEST:
        {

            add_status_line(200,ok_200_title); //返回成功状态
            if(m_file_stat.st_size!=0){
                //文件存在m_iv为2
                add_headers(m_file_stat.st_size);
                m_iv[0].iov_base = m_write_buf;
                m_iv[0].iov_len = m_write_idx;
                m_iv[1].iov_base = m_file_address;
                m_iv[1].iov_len = m_file_stat.st_size;;
                m_iv_count  = 2;
                return true;
            }
            else{
                 //如果请求的资源大小为0，则返回空白html文件
                const char* ok_string = "<html><body></body></html>";
                add_headers(strlen(ok_string));
                if(!add_content(ok_string)){
                    return false;
                }
            }
            default:
            {
                return false;
            }
        } 
        //文件不存在 除FILE_REQUEST状态外，其余状态只申请一个iovec，指向响应报文缓冲区
        m_iv[0].iov_base = m_write_buf;
        m_iv[0].iov_len = m_write_idx;
        m_iv_count = 1;
        return true;
    }
}


//对内存映射区执行munmap
void http_conn::unmap(){
    if(m_file_address){
        munmap(m_file_address,m_file_stat.st_size);
        m_file_address = 0; //地址指向空
    }
}

//写http响应
bool http_conn::write(){
    int temp = 0;
    int bytes_to_send = 0;
    int bytes_have_send = m_write_idx;
    int newadd = 0;
    if(bytes_to_send==0){
        //epoll_.mod_fd(m_epollfd,m_sockfd,EPOLLIN);
        init();
        return true;
    }

    while(1){
        //将响应报文的状态行、消息头、空行和响应正文发送给浏览器端
        //temp为发送的字节数
        temp = writev(m_sockfd,m_iv,m_iv_count);
        if(temp>0){
            bytes_have_send+= temp;//已经发送的
            newadd = bytes_have_send - m_write_idx;
        }
        if(temp<=-1){
            if(errno==EAGAIN){ //缓冲区满
                if(bytes_have_send>=m_iv[0].iov_len){
                    //第一个数组元素的成员发送完毕
                    m_iv[0].iov_len = 0;//不再发送第一个
                    m_iv[1].iov_base = m_file_address + newadd;
                    m_iv[1].iov_len = bytes_to_send;
                }
                else{
                    //继续发送第一个
                    m_iv[0].iov_base = m_write_buf + bytes_to_send;
                    m_iv[0].iov_len = bytes_to_send;
                }
                //注册写事件
                //epoll_.mod_fd(m_epollfd,m_sockfd,EPOLLOUT);
                return true;
            }
            //发送失败，且不是缓冲区的问题
            unmap();
            return false;
        }

        bytes_to_send-= temp;//更新还需要发送的字节
        
        if(bytes_to_send<=bytes_have_send){
            unmap();
            //浏览器的请求为长连接
            if(m_linger){
                init();
                //epoll_.mod_fd(m_epollfd,m_sockfd,EPOLLIN);
                return true;
            }
            else{
                //epoll_.mod_fd(m_epollfd,m_sockfd,EPOLLIN);
                return false;
            }
        }

    }

}
//往写缓冲写入待发送数据
//va_list?
bool http_conn::add_response(const char* format,...){ //用于向字符串中打印数据、数据格式用户自定义。
    if(m_write_idx>=WRITE_BUFFER_SIZE){ //写缓冲满了额
        return false;
    }
    //list //定义可变参数列表
    va_list arg_list;
    va_start(arg_list,format);
    //将数据format从可变参数列表写入缓冲区写，返回写入数据的长度 格式化数据写入m_write_buf的m_write_idx位置
    int len = vsnprintf(m_write_buf+m_write_idx,WRITE_BUFFER_SIZE-1-m_write_idx,format,arg_list);//write
    if(len>=(WRITE_BUFFER_SIZE-1-m_write_idx)){ //先释放资源
        va_end(arg_list);
        return false;
    }
    m_write_idx += len;
    va_end(arg_list); //
    return true;

}

bool http_conn::add_status_line(int status,const char* title){
    return add_response("%s %d %s\r\n","HTTP/1.1",status,title);
}





bool http_conn::add_linger(){
    return add_response("Connection: %s\r\n",(m_linger==true)?"keep-alive":"close");
}

bool http_conn::add_blank_line(){
    return add_response("%s","\r\n");
}

bool http_conn::add_content(const char* content){
    return add_response("%s",content);
}

bool http_conn::add_headers(int content_len){
    add_content_length(content_len);
    add_linger();
    add_blank_line();
}



void http_conn::process(){
    HTTP_CODE read_ret = process_read();
    if(read_ret == NO_REQUEST){ //表示请求不完整，需要继续接收请求数据
       // (m_epollfd,m_sockfd,EPOLLIN); ////注册并监听读事件
        return;
    }

    bool write_ret = process_write(read_ret);
    if(write_ret==NULL){
        close_conn();
    } 
    //epoll_.mod_fd(m_sockfd,EPOLLOUT,1); //ev就是event
}