#ifndef I04_TXT_UTIL_H
#define I04_TXT_UTIL_H

char *txtTakeUntil(char *src, char end) {
	for (int i = 0; src[i]; i++) {
		if (src[i] == end) {
			src[i] = 0;
			break;
		}
	}
	return src;
}

char *txtDropUntil(char *src, char end) {
	char *p = src;
	while (p[0]) {
		if (p[0] == end) {
			break;
		}
		p++;
	}
	return p;
}

char *txtDropWhile(char *src, char c) {
	char *p = src;
	while (*p) {
		if (*p != c) {
			break;
		}
		p++;
	}
	return p;
}

char *txtTrimEnd(char *src) {
	char *srcEnd = src;
	char *p = src;
	while (*p) {
		if (*p > ' ') {
			srcEnd = p;
		}
		p++;
	}
	srcEnd[1] = 0;
	return src;
}

char *txtTrimStart(char *src) {
	char *p = src;
	while (*p) {
		if (*p > ' ') {
			return p;
		}
		p++;
	}
	return p;
}

char *txtTrim(char *src) {
	return txtTrimEnd(txtTrimStart(src));
}

#endif