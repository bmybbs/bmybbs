# nginx 配置和使用

## 背景
1. 相对于 apache，个人 (IronBlood) 更熟悉 nginx。但是 nginx 不支持 cgi，而 www、upload 两个程序不支持 fcgi 方式运行，因此 apache 还需要保留。
2. nginx 主要负责：响应静态资源的请求；处理反向代理；负责 SSL 证书。

## 设计
1. nginx 监听 80 和 443 两个端口。
2. atthttpd 监听 8080 端口。
3. api 监听 8081 端口。
3. apache 监听 8082 端口，作为 cgi 执行器（www、upload 以及 perl 脚本）。

## 配置
### nginx

几个注意点：

1. nginx 1.3.4 以后的版本如果只监听 `[::]:80` 端口，默认只接受 IPv6 的请求，即 `ipv6only=on` 为默认配置项，因此 IPv4 和 IPv6 的地址都需要监听；
2. SSL 证书按照实际的路径配置（证书申请地址 https://gethttpsforfree.com/ ）；
3. 通常反向代理会使用 `X-Forwarded-*` 配置项，这里为了简化程序设计，与直接访问相兼容，依旧使用 `REMOTE_ADDR`。
4. 或许是 atthttpd 的缺陷（和局限性），`nginx -> atthttpd` 会接收到 reset 指令而返回 502，所以此处是 `nginx -> apache -> atthttpd`。
```
server {
	server_name bbs.xjtu.edu.cn bbs.xjtu6.edu.cn;
	listen 80;
	listen [::]:80;
	listen 443      default_server ssl;
	listen [::]:443 default_server ssl;
	ssl_certificate     /path/to/cert.pub;
	ssl_certificate_key /path/to/cert.key;

	root /home/apache/htdocs/bbs;

	location /api/ {
		proxy_set_header REMOTE_ADDR $remote_addr;
		proxy_pass http://127.0.0.1:8081/;
	}

	location ~ ^(/|/BMY(.+)?)$ {
		proxy_set_header REMOTE_ADDR $remote_addr;
		proxy_pass http://127.0.0.1:8082/$uri$is_args$args;
	}

	location /cgi-bin/ {
		proxy_set_header REMOTE_ADDR $remote_addr;
		proxy_pass http://127.0.0.1:8082/cgi-bin/;
	}

	location /attach/ {
		proxy_http_version  1.0;
		proxy_set_header REMOTE_ADDR $remote_addr;
		proxy_pass http://127.0.0.1:8082/attach/;
	}
}
```

### apache

这里就不展开配置了，参考 [\[文档\]nju09如何使用debian自带apache2.2.txt]([文档]nju09如何使用debian自带apache2.2.txt) 以及安装脚本。几个变更：
1. `/etc/apache2/ports.conf` 修改监听端口为 `127.0.0.1:8082`，且 vhost 对应变更。
2. 禁用 `ssl` 模块，启用 `remoteip` 模块。
3. 编辑 `/etc/apache2/conf-enabled/remoteip.conf` 内容为 `RemoteIPHeader REMOTE_ADDR`，这样执行 cgi 的时候才会变更为实际 IP（否则均为 `127.0.0.1`）。

