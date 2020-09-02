var left, opacity, scale;
var animating;
var animEl;

function go_forward(current, target) {
	if (animating)
		return false;

	const current_fs = $(current),
		next_fs = $(target);
	animating = true;

	next_fs.show();

	current_fs.animate({opacity: 0}, {
		step: function(now, mx) {
			scale = 1 - (1-now) * 0.2;
			left = (now * 50) + "%";
			opacity = 1 - now;
			current_fs.css({
				'transform': 'scale('+scale+')',
				'position': 'absolute'
			});
			next_fs.css({'left': left, 'opacity': opacity});
		},
		duration: 800,
		complete: function() {
			current_fs.hide();
			animating = false;
			$("#progressbar li").eq(next_fs.attr("p-index")).addClass("active");
		},
		easing: 'easeInOutBack'
	});
}

function go_backward(current, target) {
	if (animating)
		return false;

	const current_fs = $(current),
		next_fs = $(target);
	animating = true;

	next_fs.show();

	current_fs.animate({opacity: 0}, {
		step: function(now, mx) {
			scale = 0.8 + (1 - now) * 0.2;
			left = ((1-now) * 50) + "%";
			opacity = 1 - now;
			current_fs.css({'left': left});
			next_fs.css({
				'transform': 'scale('+scale+')',
				'opacity': opacity
			});
		},
		duration: 800,
		complete: function() {
			current_fs.hide();
			animating = false;
			$("#progressbar li").eq(current_fs.attr("p-index")).removeClass("active");
		},
		easing: 'easeInOutBack'
	});
}

function is_illegal_char(c) {
	return (c == "&" || c == "=" || c == "\\" || c == "/" || c == "%" || c == "?");
}

/**
 * 验证用户名
 * @return true 合法; false 不合法
 */
function verify_bmyid(bmy_id) {
	if (bmy_id == null || bmy_id.length < 2 || bmy_id.length > 12)
		return false;

	for (let i=0, l=bmy_id.length; i<l; i++) {
		if (!(bmy_id[i] >= 'a' && bmy_id[i] <= 'z') && !(bmy_id[i] >= 'A' && bmy_id[i] <= 'Z'))
			return false;
	}

	return true;
}

/**
 * 验证邮箱
 * @return
 *   -1 缺少信息
 *   -2 邮箱名过长
 *   -3 邮箱名包含不合法信息
 *   1 合法
 */
function verify_email(user, domain) {
	if (domain == null || domain == "-1") return -1;

	const d = +domain;
	if (d < 1 || d > 3) return -1;

	if (user == null || user.length == 0) return -1;
	if (user.length > 20) return -2;

	for (let i=0, l=user.length; i<l; i++) {
		if (user[i] == "@" || user[i] == " " || is_illegal_char(user[i]))
			return -3;
	}

	return 1;
}

function verify_pass(pass) {
	for (let i=0, l=pass.length; i<l; i++) {
		if (is_illegal_char(pass[i]))
			return false;
	}

	return true;
}

function startLoadingAnimation() {
	animEl.className = "anim-show";
	animEl.innerHTML = '<div class="lds-roller"><div></div><div></div><div></div><div></div><div></div><div></div><div></div><div></div></div>';
}

function stopLoadingAnimation() {
	animEl.className = "anim-hidden";
	animEl.innerHTML = "";
}

/**
 * 封装
 * @param options [object] .title, .body, .callback
 */
function show_modal(options) {
	const modal_html = `
		<div class="modal fade" id="msg-modal" data-backdrop="static" data-keyboard="false" tabindex="-1">
			<div class="modal-dialog modal-dialog-centered">
				<div class="modal-content">
					<div class="modal-header">
						<h5 class="modal-title"></h5>
						<button type="button" class="close modal-hide">
							<span aria-hidden="true">&times;</span>
						</button>
					</div>

					<div class="modal-body">
					</div>

					<div class="modal-footer" style="display: none">
					</div>
				</div>
			</div>
		</div>
	`;

	document.querySelector("#modal-container").innerHTML = modal_html;
	const footer = document.querySelector("#msg-modal .modal-footer");
	footer.innerHTML = "";
	footer.style.display = "none";

	document.querySelector("#msg-modal .modal-header h5").innerText = options.title;
	document.querySelector("#msg-modal .modal-body").innerText = options.body;

	if (typeof options.callback === "function") {
		var html = [
			'<button type="button" class="btn btn-outline-secondary modal-hide">取消</button>',
			'<button type="button" class="btn btn-secondary" id="modal-submit">确定</button>'
		].join("");

		footer.innerHTML = html;
		footer.style.display = "flex";

		$("#modal-submit").click(options.callback);
	}

	$("#msg-modal").modal('show');
	$(".modal-hide").click(() => hide_modal());
}

function hide_modal(callback) {
	$("#msg-modal").on('hidden.bs.modal', function(e) {
		document.querySelector("#modal-container").innerHTML = "";
		if (typeof callback === "function") {
			callback();
		}
	});
	$("#msg-modal").modal('hide');
}

document.addEventListener("DOMContentLoaded", function() {
	animEl = document.querySelector("div#loading-anim");
	$("#btn-resetpass")    .click(() => go_forward("#form-base", "#form-resetpass"));
	$("#btn-findacc")      .click(() => go_forward("#form-base", "#form-findacc"));
	$("#btn-doresetpass")  .click(() => go_forward("#form-resetpass", "#form-doresetpass"));
	$("#btn-dofindacc")    .click(() => go_forward("#form-findacc", "#form-dofindacc"));
	$(".go-back-resetpass").click(() => go_backward("#form-doresetpass", "#form-resetpass"));
	$(".go-back-findacc")  .click(() => go_backward("#form-dofindacc", "#form-findacc"));
	$(".go-back-base")     .click(function() {
		go_backward("#" + $(this).parent().parent().parent().attr("id"), "#form-base");
	});

	$("#btn-getcaptcha-resetpass").click(function() {
		// TODO
		const bmy_id = document.querySelector("#input-resetpass-id").value,
			user = document.querySelector("#input-resetpass-email-user").value,
			domain_el = document.querySelector("#select-resetpass-domain"),
			domain = domain_el.value;

		if (!verify_bmyid(bmy_id)) {
			show_modal({ title: "错误", body: "您输入的 BMY ID 格式不正确" });
			return;
		}

		let rc = verify_email(user, domain),
			txt;

		switch(rc) {
		case -1: txt = "缺少信息邮箱信息"; break;
		case -2: txt = "邮箱名过长"; break;
		case -3: txt = "邮箱名包含不支持的字符"; break;
		default:
			txt = [
				"您想要找回的 ID 是",bmy_id,
				"，关联的邮箱是 ", user, "@",
				domain_el.options[domain_el.selectedIndex].text,
				"，点击\"确定\"按钮发送验证码。"
			].join("");
			break;
		}

		if (rc < 0) {
			show_modal({ title: "错误", body: txt });
		} else {
			show_modal({
				title: "确认信息",
				body: txt,
				callback: function() {
					hide_modal(function() {
						startLoadingAnimation();

						$.ajax({
							url: [
								"/BMY/bbsresetpass?type=1",
								"&userid=", bmy_id,
								"&user=", user,
								"&domain=", domain
							].join(""),
							success: function(data, status, xhr) {
								stopLoadingAnimation();

								if (data.status == 0) {
									show_modal({ title: "提示", body: "您的验证码已发送，请注意查收并及时使用。"});
								} else {
									let msg;
									switch(data.status) {
									case -1: msg = "您的输入有误"; break;
									case -2: msg = "啊咧，查无此人，您输错用户id了?"; break;
									case -3: msg = "此用户并不是采用您输入的信箱认证的呀>__<"; break;
									case -4: msg = "当前无法给您发送验证码...";
										break;
									default: msg = "未知错误"; // shouldn't be here
									}

									show_modal({ title: "错误", body: msg });
								}
							},
							error: function(xhr, status, err) {
								stopLoadingAnimation();
								show_modal({ title: "出错了", body: "很抱歉兵马俑暂时无法处理您的请求，请稍后再试。" });
							}
						});
					});
				}
			});
		}
	});

	$("#btn-getcaptcha-findacc").click(function() {
		// TODO
		$("#msg-modal").modal('show');
	});

	$("#btn-post-resetpass").click(function() {
		const bmy_id = document.querySelector("#input-resetpass-id-2").value,
			pass1 = document.querySelector("#input-resetpass-pass").value,
			pass2 = document.querySelector("#input-resetpass-pass-2").value,
			code  = document.querySelector("#input-resetpass-captcha").value;

		if (!verify_bmyid(bmy_id)) {
			show_modal({ title: "错误", body: "您输入的 BMY ID 格式不正确" });
			return;
		}

		if (pass1.length == 0 || pass1.length > 13) {
			show_modal({ title: "错误", body: "您的密码为空，或者太长啦" });
			return;
		}

		if (!verify_pass(pass1)) {
			show_modal({ title: "错误", body: "请勿使用 \"&=\\/%?\" 这些字符" });
			return;
		}

		if (pass1 != pass2) {
			show_modal({ title: "错误", body: "两次密码不一致" });
			return;
		}

		if (code.length != 5) {
			show_modal({ title: "错误", body: "您的验证码格式不正确" });
			return;
		}

		show_modal({
			title: "确认信息",
			body: "您确认按照表单中的内容重置密码吗？",
			callback: function() {
				hide_modal(function() {
					startLoadingAnimation();

					$.ajax({
						url: [
							"/BMY/bbsresetpass?type=2",
							"&userid=", bmy_id,
							"&pass1=", pass1,
							"&pass2=", pass2,
							"&code=", code
						].join(""),
						success: function(data, status, xhr) {
							stopLoadingAnimation();
							if (data.status == 0) {
								show_modal({ title: "提示", body: "你的密码已修改，请使用新密码登录。" });
							} else {
								let msg;
								switch(data.status) {
								case -1: msg = "您的输入有误"; break;
								case -2: msg = "啊咧，查无此人，您输错用户id了?"; break;
								case -3: msg = "您的验证码好像不对嘛..."; break
								default: msg = "未知错误"; // shouldn't be here
								}

								show_modal({ title: "错误", body: msg });
							}
						},
						error: function(xhr, status, err) {
							stopLoadingAnimation();
							show_modal({ title: "出错了", body: "很抱歉兵马俑暂时无法处理您的请求，请稍后再试。" });
						}
					});
				});
			}
		});
	});

	$("#btn-post-query").click(function() {
		// TODO
		$("#msg-modal").modal('show');
	});
});
