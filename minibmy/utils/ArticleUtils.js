import { DOMParser, XMLSerializer } from "xmldom"

export const minibmyFormatArticle = content => {
	const doc = new DOMParser().parseFromString(content, "text/html");

	// 处理引言 blockquote
	const bq_arr = [].slice.call(doc.getElementsByTagName("blockquote"));
	bq_arr.forEach(x => {
		x.setAttribute("class", "blockquote")
	});

	// 处理代码块
	const pre_arr = [].slice.call(doc.getElementsByTagName("pre"));
	pre_arr.forEach(x => {
		x.setAttribute("class", "pre");
	});

	return new XMLSerializer().serializeToString(doc);
}

