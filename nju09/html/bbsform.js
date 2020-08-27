var DOM_F_REG,
	DOM_F_CAP;

function switch_view(status) {
	// status == 0, CAPTCHA_OK
	if (status == 0) {
		DOM_F_REG.classList.add("hidden");
		DOM_F_CAP.classList.remove("hidden");
	} else {
		DOM_F_REG.classList.remove("hidden");
		DOM_F_CAP.classList.add("hidden");
	}
};

document.addEventListener("DOMContentLoaded", function() {
	DOM_F_REG = document.querySelector("#form-reg");
	DOM_F_CAP = document.querySelector("#form-cap");

	switch_view(cap_status);
});

