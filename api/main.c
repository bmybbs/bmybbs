#include <unistd.h>
#include <signal.h>
#include <onion/onion.h>
#include "config.h"
#include "apilib.h"
#include "api.h"

static onion *o = NULL;

static void shutdown_server(int _)
{
	(void) _;
	if (o)
		onion_listen_stop(o);
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	seteuid(BBSUID);
	setuid(BBSUID);
	setgid(BBSGID);

	chdir(MY_BBS_HOME);

	if(shm_init()<0)
		return -1;

	signal(SIGINT, shutdown_server);
	signal(SIGTERM, shutdown_server);

	o = onion_new(O_POOL);
	onion_set_max_threads(o, 32);

	onion_set_timeout(o, 5000);
	onion_set_hostname(o, "127.0.0.1");
	onion_set_port(o, "8081");

	onion_url *urls=onion_root_url(o);
	onion_url_add(urls, "", api_error);

	onion_url_add(urls, "^user/query$", api_user_query);
	onion_url_add(urls, "^user/login$", api_user_login);
	onion_url_add(urls, "^user/logout$", api_user_logout);
	onion_url_add(urls, "^user/checksession$", api_user_check_session);
	onion_url_add(urls, "^user/articlequery$", api_user_articlequery);
	onion_url_add(urls, "^user/friends/list$", api_user_friends_list);
	onion_url_add(urls, "^user/friends/add$", api_user_friends_add);
	onion_url_add(urls, "^user/friends/del$", api_user_friends_del);
	onion_url_add(urls, "^user/rejects/list$", api_user_rejects_list);
	onion_url_add(urls, "^user/rejects/add$", api_user_rejects_add);
	onion_url_add(urls, "^user/rejects/del$", api_user_rejects_del);
	onion_url_add(urls, "^user/autocomplete$", api_user_autocomplete);
	onion_url_add(urls, "^oauth/2fa_get_key$", api_oauth_2fa_get_key);
	onion_url_add(urls, "^oauth/2fa_get_code$", api_oauth_2fa_get_code);
	onion_url_add(urls, "^oauth/2fa_check_code$", api_oauth_2fa_check_code);
	onion_url_add(urls, "^oauth/remove_wx$", api_oauth_remove_wx);
	onion_url_add(urls, "^oauth/login$", api_oauth_login);
	onion_url_add(urls, "^article/list$", api_article_list);
	onion_url_add(urls, "^article/getHTMLContent$", api_article_getHTMLContent);
	onion_url_add(urls, "^article/getRAWContent$", api_article_getRAWContent);
	onion_url_add(urls, "^article/getContent$", api_article_getContent);
	onion_url_add(urls, "^article/post$", api_article_post);
	onion_url_add(urls, "^article/reply$", api_article_reply);
	onion_url_add(urls, "^board/list$", api_board_list);
	onion_url_add(urls, "^board/info$", api_board_info);
	onion_url_add(urls, "^board/fav/add$", api_board_fav_add);
	onion_url_add(urls, "^board/fav/del$", api_board_fav_del);
	onion_url_add(urls, "^board/fav/list$", api_board_fav_list);
	onion_url_add(urls, "^board/autocomplete$", api_board_autocomplete);
	onion_url_add(urls, "^meta/loginpics", api_meta_loginpics);
	onion_url_add(urls, "^mail/list$", api_mail_list);
	onion_url_add(urls, "^mail/getHTMLContent$", api_mail_getHTMLContent);
	onion_url_add(urls, "^mail/getRAWContent$", api_mail_getRAWContent);
	onion_url_add(urls, "^mail/post$", api_mail_send);
	onion_url_add(urls, "^mail/reply$", api_mail_reply);
	onion_url_add(urls, "^attach/show$", api_attach_show);
	onion_url_add(urls, "^attach/list$", api_attach_list);
	onion_url_add(urls, "^attach/get$", api_attach_get);
	onion_url_add(urls, "^attach/upload$", api_attach_upload);
	onion_url_add(urls, "^attach/delete$", api_attach_delete);
	onion_url_add(urls, "^notification/list$", api_notification_list);
	onion_url_add(urls, "^notification/del$", api_notification_del);
	onion_url_add(urls, "^search/content$", api_search_content);
	onion_url_add(urls, "^subscription/list$", api_subscription_list);

	onion_listen(o);

	onion_free(o);
	return 0;
}
