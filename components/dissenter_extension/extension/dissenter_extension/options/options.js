var BROWSER_CONFIG = {"name":"Google Chrome","slug":"chrome","version":"0.1.9"};/* BROWSERS */
var BROWSER_CHROME_SLUG = 'chrome';
var BROWSER_FIREFOX_SLUG = 'firefox';
var BROWSER_EDGE_SLUG = 'edge';
var BROWSER_SAFARI_SLUG = 'safari';

/* BASE URI FOR DISSENTER EXTENSION */
var DISSENTER_HOME_PAGE_URI = "https://identity.faith";
var BASE_URI = 'https://identity.faith/discussion/begin-extension?url=';
var COMMENT_COUNT_URI = "https://identity.faith/notification/comment-count?url=";

/* ACTIONS */
var BACKGROUND_ACTION_OPEN_POPUP = 'open_popup';
var BACKGROUND_ACTION_GET_KEY = 'get_key';
var BACKGROUND_ACTION_SET_KEY = 'set_key';
var BACKGROUND_ACTION_SET_BADGE = 'set_badge';
var BACKGROUND_ACTION_TAB_UPDATED = 'tab_updated';

/* COLORS */
var COLOR_GAB_GREEN = '#21cf7b';

/* SEARCH ENGINES */
var SEARCH_ENGINES = [
    {
        name: "DuckDuckGo",
        url: "https://duckduckgo.com/?q=",
        icon: "duckduckgo.svg"
    },
    {
        name: "Startpage",
        url: "https://www.startpage.com/do/search?q=",
        icon: "startpage.png"
    },
    {
        name: "Qwant",
        url: "https://www.qwant.com/?q=",
        icon: "qwant.svg"
    },
    {
        name: "Bing",
        url: "https://www.bing.com/search?q=",
        icon: "bing.svg"
    },
    {
        name: "Yahoo!",
        url: "https://search.yahoo.com/search?p=",
        icon: "yahoo.svg"
    },
    {
        name: "Google",
        url: "https://www.google.com/search?q=",
        icon: "google.svg"
    }
];

/* STORAGE */
var STORAGE_BASE = 'gab_dissenter_extension_data';

var TWITTER_BUTTONS_ENABLED = 'twitter_buttons_enabled';
var REDDIT_BUTTONS_ENABLED = 'reddit_buttons_enabled';
var YOUTUBE_BUTTONS_ENABLED = 'youtube_buttons_enabled';
var WINDOW_SIDEBAR_UNAVAILABLE_ENABLED = 'window_sidebar_unavailable_enabled';
var WEBSITE_COMMENT_BADGE_ENABLED = 'website_comment_badge_enabled';
var DISSENT_DISQUS_BUTTONS_ENABLED = 'dissent_disqus_buttons_enabled';
var WIKIPEDIA_BUTTONS_ENABLED = 'wikipedia_buttons_enabled';
var CUSTOM_NEW_TAB_ENABLED = 'custom_new_tab_enabled';

/* NEW TAB */
var NT_DEFAULT_SEARCH_ENGINE = 'nt_default_search_engine';

var NT_TOP_SITES_ENABLED = 'nt_top_sites_enabled';
var NT_TOP_SITES_LIMIT = 'nt_top_sites_limit';
var NT_TOP_SITES_SIZE = 'nt_top_sites_size';
var NT_TOP_SITES_SHAPE = 'nt_top_sites_shape';
var NT_TOP_SITES_HIGHLIGHT = 'nt_top_sites_highlight';
var NT_TOP_SITES_SHOW_TITLE = 'nt_top_sites_show_title';

var NT_DATETIME_SHOW_DATE = 'nt_datetime_show_date';
var NT_DATETIME_SHOW_TIME = 'nt_datetime_show_time';

var NT_COLORS_SEARCH = 'nt_colors_search';
var NT_COLORS_TEXT = 'nt_colors_text';

var NT_BACKGROUND_SOLID_COLOR = 'nt_background_solid_color';
var NT_BACKGROUND_IMAGE = 'nt_background_image';
var NT_BACKGROUND_RANDOM_GRADIENT = 'nt_background_random_gradient';
var NT_BACKGROUND_IMAGE_URL = 'nt_background_image_url';

var NT_DISSENTER_ENABLED = 'nt_dissenter_enabled';
var NT_DISSENTER_DEFAULT_TAB = 'nt_dissenter_default_tab';

var NT_DISSENTER_PINS = 'nt_dissenter_pins';
var NT_DISSENTER_HIDE_TIPS = 'nt_dissenter_hide_tips';

/* STORAGE DEFAULTS */
var STORAGE_DEFAULT_PARAMS = {};
STORAGE_DEFAULT_PARAMS[TWITTER_BUTTONS_ENABLED] = true;
STORAGE_DEFAULT_PARAMS[REDDIT_BUTTONS_ENABLED] = true;
STORAGE_DEFAULT_PARAMS[YOUTUBE_BUTTONS_ENABLED] = true;
STORAGE_DEFAULT_PARAMS[WINDOW_SIDEBAR_UNAVAILABLE_ENABLED] = true;
STORAGE_DEFAULT_PARAMS[WEBSITE_COMMENT_BADGE_ENABLED] = false;
STORAGE_DEFAULT_PARAMS[DISSENT_DISQUS_BUTTONS_ENABLED] = false;
STORAGE_DEFAULT_PARAMS[WIKIPEDIA_BUTTONS_ENABLED] = true;
STORAGE_DEFAULT_PARAMS[CUSTOM_NEW_TAB_ENABLED] = true;
STORAGE_DEFAULT_PARAMS[NT_DEFAULT_SEARCH_ENGINE] = SEARCH_ENGINES[0];
STORAGE_DEFAULT_PARAMS[NT_TOP_SITES_ENABLED] = true;
STORAGE_DEFAULT_PARAMS[NT_TOP_SITES_LIMIT] = 20;
STORAGE_DEFAULT_PARAMS[NT_TOP_SITES_SIZE] = "md";
STORAGE_DEFAULT_PARAMS[NT_TOP_SITES_SHAPE] = "circle";
STORAGE_DEFAULT_PARAMS[NT_TOP_SITES_HIGHLIGHT] = "light";
STORAGE_DEFAULT_PARAMS[NT_TOP_SITES_SHOW_TITLE] = true;
STORAGE_DEFAULT_PARAMS[NT_DATETIME_SHOW_DATE] = true;
STORAGE_DEFAULT_PARAMS[NT_DATETIME_SHOW_TIME] = true;
STORAGE_DEFAULT_PARAMS[NT_COLORS_SEARCH] = "white";
STORAGE_DEFAULT_PARAMS[NT_COLORS_TEXT] = "white";
STORAGE_DEFAULT_PARAMS[NT_BACKGROUND_SOLID_COLOR] = "";
STORAGE_DEFAULT_PARAMS[NT_BACKGROUND_IMAGE] = "";
STORAGE_DEFAULT_PARAMS[NT_BACKGROUND_IMAGE_URL] = "../assets/images/defiant-bg1.jpg";
STORAGE_DEFAULT_PARAMS[NT_BACKGROUND_RANDOM_GRADIENT] = false;
STORAGE_DEFAULT_PARAMS[NT_DISSENTER_ENABLED] = false;
STORAGE_DEFAULT_PARAMS[NT_DISSENTER_DEFAULT_TAB] = "home";
STORAGE_DEFAULT_PARAMS[NT_DISSENTER_PINS] = {
    "page1": [
        {url:"https://telosgreen.org", title:"Telos Coin"},
        {url:"https://bitdorado.exchange", title:"Bitdorado exchange"},
        {url:"https://governance.rocks", title:"governance.rocks"},
        {url:"https://bitcoin-subsidium.org", title:"Bitcoin Subsidium"},
        {url:"https://libra-ai.org", title:"Free Libra"}
    ]
};
STORAGE_DEFAULT_PARAMS[NT_DISSENTER_HIDE_TIPS] = false;

var STORAGE_KEY_ALL = 'all';

/**
 * @description Internal helper to check if parameter is an object
 * @function isObject
 * @param {*} obj
 * @returns {boolean}
 */
var isObject = function(obj) {
    if (isArray(obj) || isDate(obj)) return false;
    return (obj !== null && typeof obj === 'object');
};

/**
 * @description Internal helper to check if parameter is a string
 * @function isString
 * @param {*} str
 * @returns {boolean}
 */
var isString = function(str) {
    return (typeof str === 'string' || str instanceof String);
};

/**
 * @description Internal helper to check if parameter is a date
 * @function isDate
 * @param {*} date
 * @returns {boolean}
 */
var isDate = function(date) {
    if (isString(date) || isArray(date) || date == undefined || date == null) return false;
    return (date && Object.prototype.toString.call(date) === "[object Date]" && !isNaN(date));
};

/**
 * @description Internal helper to check if parameter is an array
 * @function isArray
 * @param {*} arr
 * @returns {boolean}
 */
var isArray = function(arr) {
    return Array.isArray(arr);
};

/**
 * @description Internal helper to check if DOMContentLoaded
 * @function ready
 * @param {Function} fn
 */
function ready(fn) {
    var d = document;
    (d.readyState == 'loading') ? d.addEventListener('DOMContentLoaded', fn): fn();
};

/**
 * @description Internal helper to get value from query string
 * @function getQueryStringValue
 * @param {String} key
 * @returns {String|null}
 */
function getQueryStringValue(key) {
    return decodeURIComponent(window.location.search.replace(new RegExp("^(?:.*[&\\?]" + encodeURIComponent(key).replace(/[\.\+\*]/g, "\\$&") + "(?:\\=([^&]*))?)?.*$", "i"), "$1"));
};

function getDissenterDLogoAsSVG(height, width, fillColor, backgroundColor) {
    //Create "g" dissent icon
    var svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
    svg.style.setProperty("height", height, "important");
    svg.style.setProperty("width", width, "important");
    svg.setAttribute("version", "1.1");
    svg.setAttribute("xmlns", "http://www.w3.org/2000/svg");
    svg.setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    svg.setAttribute("viewBox", "0 0 500 500");

    var g = document.createElementNS("http://www.w3.org/2000/svg", "g");
    g.style.setProperty("transform", "translate(17, 37)", "important");

    var solid = document.createElementNS("http://www.w3.org/2000/svg", "path");
    solid.setAttribute("d", "M244.885937,0 L193.657763,0 L65.5018905,0 L45.5665325,0 C20.4992438,0 0,20.5016933 0,45.5466667 L0,381.453333 C0,406.504 20.4992438,427 45.5665325,427 L65.5018905,427 L193.657763,427 L244.885937,427 C389.348933,427 467,326.3134 467,213.5 C467,100.6866 393.563837,0 244.885937,0");
    solid.setAttribute("fill", fillColor);

    var line1 = document.createElementNS("http://www.w3.org/2000/svg", "path");
    line1.setAttribute("d", "M309.275885,155 L121.729792,155 C115.246045,155 110,149.623395 110,143.002906 C110,136.376605 115.246045,131 121.729792,131 L309.275885,131 C315.759633,131 321,136.376605 321,143.002906 C321,149.623395 315.759633,155 309.275885,155");
    line1.setAttribute("fill", backgroundColor);

    var line2 = document.createElementNS("http://www.w3.org/2000/svg", "path");
    line2.setAttribute("d", "M309.275885,225 L121.729792,225 C115.246045,225 110,219.623395 110,213.002906 C110,206.376605 115.246045,201 121.729792,201 L309.275885,201 C315.759633,201 321,206.376605 321,213.002906 C321,219.623395 315.759633,225 309.275885,225");
    line2.setAttribute("fill", backgroundColor);

    var line3 = document.createElementNS("http://www.w3.org/2000/svg", "path");
    line3.setAttribute("d", "M121.78718,296 C115.271711,296 110,290.623395 110,284.002906 C110,277.376605 115.271711,272 121.78718,272 L247.218525,272 C253.733994,272 259,277.376605 259,284.002906 C259,290.623395 253.733994,296 247.218525,296 L121.78718,296 Z");
    line3.setAttribute("fill", backgroundColor);

    g.appendChild(solid);
    g.appendChild(line1);
    g.appendChild(line2);
    g.appendChild(line3);

    svg.appendChild(g);

    return svg;
};

function addManyClasses(elem, classes) {
    if (!isArray(classes)) return false;

    for (var i = 0, length = classes.length; i < length; i++) {
        elem.classList.add(classes[i]);
    };
};

function removeManyClasses(elem, classes) {
    if (!isArray(classes)) return false;
    
    for (var i = 0, length = classes.length; i < length; i++) {
      elem.classList.remove(classes[i]);
  };
};

/**
 * @description - Options
 */
var Options = function() {

    //Declare scope
    var scope = this;

    //

    var optionalPermissions = {
        permissions: ["tabs"]
    };

    var checkboxData = [
        {
            element: document.getElementById('twitter-enabled'),
            key: TWITTER_BUTTONS_ENABLED,
        },
        {
            element: document.getElementById('reddit-enabled'),
            key: REDDIT_BUTTONS_ENABLED,
        },
        {
            element: document.getElementById('youtube-enabled'),
            key: YOUTUBE_BUTTONS_ENABLED,
        },
        {
            element: document.getElementById('window-sidebar-enabled'),
            key: WINDOW_SIDEBAR_UNAVAILABLE_ENABLED,
        },
        {
            element: document.getElementById('comment-badge-enabled'),
            key: WEBSITE_COMMENT_BADGE_ENABLED,
        },
        {
            element: document.getElementById('disqus-dissent-enabled'),
            key: DISSENT_DISQUS_BUTTONS_ENABLED,
        },
        {
            element: document.getElementById('wikipedia-enabled'),
            key: WIKIPEDIA_BUTTONS_ENABLED,
        },
        {
            element: document.getElementById('custom-new-tab-enabled'),
            key: CUSTOM_NEW_TAB_ENABLED,
        }
    ];

    function setKeyValue(checkboxBlock) {

        //Send message
        chrome.runtime.sendMessage({
            action: BACKGROUND_ACTION_SET_KEY,
            key: checkboxBlock.key,
            value: checkboxBlock.element.checked
        });
    };

    for (var i = 0; i < checkboxData.length; i++) {
        var checkboxBlock = checkboxData[i];
        var key = checkboxBlock.key;
        checkboxBlock.element.onchange = setKeyValue.bind(null, checkboxBlock);
    };

    //Global functions

    /**
     * @description - Function to call on options instantiation
     * @function scope.init
     */
    scope.init = function() {
        //Send message to browser to get config data
        chrome.runtime.sendMessage({
            action: BACKGROUND_ACTION_GET_KEY,
            key: STORAGE_KEY_ALL
        }, function(data) {
            if (!data) return false;

            for (var i = 0; i < checkboxData.length; i++) {
                checkboxData[i].element.checked = data[checkboxData[i].key];
            };
        });
    };
};

/**
 * @description - On options DOMContentLoaded
 */
document.addEventListener('DOMContentLoaded', function() {
    //Init new options page
    var options = new Options();
    options.init();
});
