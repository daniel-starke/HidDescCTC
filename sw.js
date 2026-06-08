const version = "sw-1.0.0::";

/* A list of local resources we always want to be cached. */
const offlineFundamentals = [
	"index.html",
	"./", /* Alias for index.html */
	//"sw.js",
	"manifest.webmanifest",
	"assets/appicon.png",
	"assets/codejar.js",
	"assets/favicon.ico",
	"assets/HidWebCompiler.js",
	"assets/HidWebCompiler.wasm",
	"assets/linenumbers.js",
	"assets/prism.css",
	"assets/prism.js"
];

/* The install handler takes care of pre-caching the resources we always need. */
self.addEventListener("install", function(event) {
	console.log("install");
	event.waitUntil(
		/* The caches built-in is a promise-based API that helps you cache responses,
		 * as well as finding and deleting them. */
		caches.open(version + "fundamentals")
		.then(function(cache) {
			return cache.addAll(offlineFundamentals);
		})
		.then(function () {
			self.skipWaiting();
		})
	);
});

/* The activate handler takes care of cleaning up old caches. */
self.addEventListener("activate", function(event) {
	console.log("activate");
	event.waitUntil(
		caches.keys().then(function (cacheNames) {
			return Promise.all(
				cacheNames
					.filter(function (currentCaches) {
						return !currentCaches.startsWith(version);
					})
					.map(function (cacheToDelete) {
						return caches.delete(cacheToDelete);
					})
			);
		}).then(function () {
			self.clients.claim();
		})
	);
});

/* The fetch handler serves responses for same-origin resources from a cache.
 * If no response is found, it populates the runtime cache with the response
 * from the network before returning it to the page. */
self.addEventListener("fetch", function(event) {
	console.log("fetch");
	/* Skip cross-origin requests. */
	if ( event.request.url.startsWith(self.location.origin) ) {
		event.respondWith(
			caches.match(event.request).then(function (cachedResponse) {
				if ( cachedResponse ) {
					return cachedResponse;
				}
				return caches.open(version + "responses").then(function (cache) {
					return fetch(event.request).then(function (response) {
						/* Put a copy of the response in the runtime cache. */
						return cache.put(event.request, response.clone()).then(function () {
							return response;
						});
					});
				});
			})
		);
	}
});
