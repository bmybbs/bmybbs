var left, opacity, scale;
var animating;

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

document.addEventListener("DOMContentLoaded", function() {
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
		$("#msg-modal").modal('show');
	});

	$("#btn-getcaptcha-findacc").click(function() {
		// TODO
		$("#msg-modal").modal('show');
	});

	$("#btn-post-resetpass").click(function() {
		// TODO
		$("#msg-modal").modal('show');
	});

	$("#btn-post-query").click(function() {
		// TODO
		$("#msg-modal").modal('show');
	});
});
