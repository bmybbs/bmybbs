let mathJaxReady;

export async function ensureMathJax() {
	if (!mathJaxReady) {
		window.MathJax = {
			tex: {
				inlineMath: [['$', '$'], ['\\(', '\\)']],
			},
			svg: {
				fontCache: "global",
			},
			startup: {
				typeset: false,
			},
		};

		mathJaxReady = import("@mathjax/src/bundle/tex-svg.js").then(async () => {
			await window.MathJax.startup.promise;
			return window.MathJax;
		});
	}

	return mathJaxReady;
}

export async function typesetMath(elements) {
	const MathJax = await ensureMathJax();
	await MathJax.typesetPromise(Array.isArray(elements) ? elements : [elements]);
}
