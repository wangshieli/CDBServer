<?php
	/**
	 * 接口调用示例代码(PHP)
	 * @Title: demo.php
	 * @author 陈杨
	 * @date 2017年2月8日 上午9:19:22
	 * @version V1.0
	 * @Copyright 中国电信股份有限公司物联网分公司
	 * @description API开发手册登录物联网自管理门户->在线文档查看
	*/
	require_once './DesUtils.php';
	
	//具体接口参数需参照自管理门户在线文档
	$access_number="14914000000"; //物联网卡号(149或10649号段)
	$user_id="test";		//用户名
	$password="test"; 		//密码
	$method = "queryPakage";  //接口名-套餐使用量查询
	$arr=array($access_number,$user_id,$password,$method);//加密数组，数组所需参数根据对应的接口文档
	
	//key值指密钥，由电信提供，每个客户都有对应的key值，key值平均分为三段如下：
    $key1 = "abc";
    $key2 = "def";
    $key3 = "ghi";
    $des=new DesUtils(); //加密工具类实例化
	$passwordEnc=$des->strEnc($password,$key1,$key2,$key3);  //密码加密 
	echo "密码加密结果:".$passwordEnc;
	echo '</br>';
	//密码加密结果：441894168BD86A2CC
	
	$arrOrder=$des->naturalOrdering($arr); //加密数组元素排序
    $sign = $des->strEnc($arrOrder,$key1,$key2,$key3);
	echo "sign加密结果:".$sign;
	echo '</br>';	//sign加密结果：45E8B9924DE397A8F7E5764767810CF774CC7E1685BA702C9C4C367EFDAE5D932B37C0C8F0F8EB0CAD6372289F407CA941894168BD86A2CC32E5804EA05BAA5099649468B9418E52
	
	$passwordDec=$des->strDec($passwordEnc,$key1,$key2,$key3);//密码解密
	echo "密码解密结果:".$passwordDec;
	echo '</br>';
    //密码解密结果 :test
		
	$signDec=$des->strDec($sign,$key1,$key2,$key3); //sign解密
	echo "sign解密结果:".$signDec;
	echo '</br>';
	//sign解密结果：14914000000,queryPakage,test,test		
?>