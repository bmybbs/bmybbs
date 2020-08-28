var DOM_F_REG,
	DOM_F_CAP;

function myFetch(type) {
	return fetch("bbsform?type=" + type)
		.then(response => response.json());
}

function switch_view(status) {
	// status == 0, CAPTCHA_OK
	if (status == 0) {
		DOM_F_REG.classList.add("hidden");
		DOM_F_CAP.classList.remove("hidden");
	} else {
		DOM_F_REG.classList.remove("hidden");
		DOM_F_CAP.classList.add("hidden");
	}
}

function update_form_email() {
	var email = document.querySelector("#cap-email").innerText, arr, idx;
	if (email.length > 0) {
		arr = email.split("@");

		if (arr.length == 2) {
			document.querySelector("input[name=user]").value = arr[0];

			idx = 1;
			if (arr[1] == "xjtu.edu.cn") idx = 1;
			else if (arr[1] == "stu.xjtu.edu.cn") idx = 2;
			else if (arr[1] == "mail.xjtu.edu.cn") idx = 3;
			document.querySelector("select[name=popserver]").value = idx;
		}
	}
}

function get_status(status) {
	let msg;
	switch (status) {
	case -1: msg = "抱歉遇到系统故障，请联系站长"; break;
	case -3: msg = "您好像没有验证码，请重新提交表单"; break;
	case 1: msg = "您的验证码已失效"; break;
	case 3: msg = "您的验证码已超时，请重新提交表单"; break;
	default : msg = "";
	}
	return msg;
}

document.addEventListener("DOMContentLoaded", function() {
	DOM_F_REG = document.querySelector("#form-reg");
	DOM_F_CAP = document.querySelector("#form-cap");

	var form = document.querySelector("#form-reg form"),
		modal = document.querySelector("#myModal"),
		span = document.querySelector(".close"),
		p = document.querySelector("#myModal p");

	switch_view(cap_status);
	update_form_email();

	span.addEventListener("click", function() {
		modal.style.display = "none";
	});

	window.addEventListener("click", function(event) {
		if (event.target == modal) {
			modal.style.display = "none";
		}
	});

	document.querySelector("#btn-reg-submit").addEventListener("click", function() {
		modal.style.display = "block";
		p.innerText = "您的验证信息正在被提交，请耐心等待...";
		form.submit();
	});

	document.querySelector("#btn-reg-reset").addEventListener("click", function() {
		form.reset();
		update_form_email();
	});

	document.querySelector("#btn-cap-submit").addEventListener("click", function() {
		modal.style.display = "block";
		p.innerText = "正在校验您的信息，请耐心等待...";

		myFetch("2&code=" + document.querySelector("input[name=captcha]").value).then(function(data) {
			modal.style.display = "block";
			if (data.status == 0) {
				p.innerText = "您已成功通过验证，请刷新网页更新您的资料";
			} else {
				p.innerText = get_status(data.status);
			}
		});
	});

	document.querySelector("#btn-cap-resent").addEventListener("click", function() {
		modal.style.display = "block";
		p.innerText = "您的请求已发送，请耐心等待...";

		myFetch("3").then(function(data) {
			modal.style.display = "block";
			if (data.status == 1) {
				// 已发送
				p.innerText = "请查收邮箱，并在 15 分钟内输入验证码";
			} else {
				p.innerText = "无法验证您的邮箱，请检查认证信息";
			}
		});
	});

	document.querySelector("#btn-reg-captcha").addEventListener("click", function() {
		switch_view(0);
	});

	document.querySelector("#btn-cap-update").addEventListener("click", function() {
		switch_view(1);
	});
});

