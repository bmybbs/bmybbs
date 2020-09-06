From cmtsai1@summer.nchu.edu.tw  Sat May 14 17:22:58 1994
Received: from summer.nchu.edu.tw (summer.nchu.edu.tw [140.120.3.3]) by phoenix.csie.nctu.edu.tw (8.6.9/8.6.4) with SMTP id RAA12351 for <skhuang@csie.nctu.edu.tw>; Sat, 14 May 1994 17:22:57 +0800
Message-Id: <199405140922.RAA12351@phoenix.csie.nctu.edu.tw>
Received: by summer.nchu.edu.tw
	(1.37.109.4/16.2) id AA05160; Sat, 14 May 94 17:28:16 +0800
From: Chi-Ming Tsai <cmtsai1@summer.nchu.edu.tw>
Subject: Re: the new hanzi-convert table :)
To: skhuang@csie.nctu.edu.tw (Shih-Kun Huang) (Shih-Kun Huang)
Date: Sat, 14 May 94 17:28:16 EAT
In-Reply-To: <199405060919.RAA21155@phoenix.csie.nctu.edu.tw>; from "Shih-Kun Huang" at May 6, 94 5:17 pm
Mailer: Elm [revision: 70.85]
Status: O

> Another problem is that I can't convert big5 into jis code.
> Have you successfully done it ?
找到bug 了!! 在 line 285 of sinocode.c:
  index = ((ch1-161)*157)+ch2-63-(ch1 < 161)?0:34;
                                 ^^^^^^^^^^^^^^^^
改成
  index = ((ch1-161)*157)+ch2-63-((ch2 < 161)?0:34);
                                 ^^^^^^^^^^^^^^^^^^
就可以了。
哈哈哈, 我們來跟日本人聊天吧!!!
(可以問問他們有關華航空難的調查結果、亂馬1/2 最新發展．．．
．．．．．嗯。就是這樣。)
辛苦了!! :)

