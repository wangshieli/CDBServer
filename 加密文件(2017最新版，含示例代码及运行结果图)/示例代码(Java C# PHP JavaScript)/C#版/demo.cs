/**
 * 接口调用示例代码(C#)
 * @Title: demo.cs
 * @author 陈杨
 * @date 2017年2月8日 下午15:11:10
 * @version V1.0
 * @Copyright 中国电信股份有限公司物联网分公司
 * @description API开发手册登录物联网自管理门户->在线文档查看
*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections;
namespace demo
{
    class Program
    {
        static void Main(string[] args)
        {
            //具体接口参数需参照自管理门户在线文档
            string access_number = "14914000000";  //物联网卡号(149或10649号段)
            string user_id = "test";     //用户名
            string password = "test";    //密码
            string method = "queryPakage";  //接口名-套餐使用量查询

            string[] arr = { access_number, user_id, password, method}; //加密数组，数组所需参数根据对应的接口文档

            //key值指密钥，由电信提供，每个客户都有对应的key值，key值平均分为三段如下：
            string key1 = "abc";
            string key2 = "def";
            string key3 = "ghi";

            DesUtils des = new DesUtils(); //加密工具类实例化
            string a="A325A844847AAF3DDA5EEC90686AFC9A9AC9A8C7324C64198EDAD0861FDC6762727E5537B736279909B9472366E2BB868AC4966FF7ADA449D9AD5F66DE97915E5BF787A7280CB745D550074CC9FDF93342B43ED72D0389553F6C67408381563A3978A9E246FC1522";

            string passwordEnc = des.strEnc(password,key1,key2,key3);  //密码加密 
		    Console.WriteLine("密码加密结果:"+passwordEnc);
		    //密码加密结果：441894168BD86A2CC
		    
            string sign = des.strEnc(des.naturalOrdering(arr),key1,key2,key3); //生成sign加密值
		    Console.WriteLine("sign加密结果:"+sign);
		   //sign加密结果：45E8B9924DE397A8F7E5764767810CF774CC7E1685BA702C9C4C367EFDAE5D932B37C0C8F0F8EB0CAD6372289F407CA941894168BD86A2CC32E5804EA05BAA5099649468B9418E52
		
		   string passwordDec = des.strDec(passwordEnc,key1,key2,key3);//密码解密
		   Console.WriteLine("密码解密结果:"+passwordDec);
           //密码解密结果 :test
		    
		   string signDec = des.strDec(sign,key1,key2,key3); //sign解密
           Console.WriteLine("sign解密结果:" + signDec);
		   //sign解密结果：14914000000,queryPakage,test,test
           Console.ReadLine();
        }
    }
}
