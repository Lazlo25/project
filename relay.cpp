// relay.cpp: a carrier pigeon that flies GitHub gossip to Slack. Lambda pays it in breadcrumbs.

#include <curl/curl.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>

struct Pigeon {
    std::string ticket;
    std::string crumbs;
    std::string scoop;
    std::string scroll;
    std::string squawk;
    bool grounded = false;
};

using Trick = bool (*)(Pigeon&);

Pigeon& operator|(Pigeon& bird, Trick trick) {
    if (!bird.grounded) bird.grounded = !trick(bird);
    return bird;
}

static size_t hoard(char* bits, size_t size, size_t count, void* nest) {
    static_cast<std::string*>(nest)->append(bits, size * count);
    return size * count;
}

// yeet: throws bytes at a URL and pockets whatever bounces back
static std::string yeet(const std::string& where, const std::string* payload,
                        std::string* gossip = nullptr, long patience = 0) {
    std::string loot;
    CURL* slingshot = curl_easy_init();
    curl_easy_setopt(slingshot, CURLOPT_URL, where.c_str());
    curl_easy_setopt(slingshot, CURLOPT_WRITEFUNCTION, hoard);
    curl_easy_setopt(slingshot, CURLOPT_WRITEDATA, &loot);
    if (gossip) {
        curl_easy_setopt(slingshot, CURLOPT_HEADERFUNCTION, hoard);
        curl_easy_setopt(slingshot, CURLOPT_HEADERDATA, gossip);
    }
    curl_slist* stickers = nullptr;
    if (payload) {
        stickers = curl_slist_append(stickers, "Content-Type: application/json");
        curl_easy_setopt(slingshot, CURLOPT_HTTPHEADER, stickers);
        curl_easy_setopt(slingshot, CURLOPT_POSTFIELDS, payload->c_str());
    }
    curl_easy_setopt(slingshot, CURLOPT_TIMEOUT, patience);
    CURLcode oops = curl_easy_perform(slingshot);
    if (oops != CURLE_OK) loot = std::string("curl error: ") + curl_easy_strerror(oops);
    curl_slist_free_all(stickers);
    curl_easy_cleanup(slingshot);
    return loot;
}

static std::string bubbleWrap(const std::string& fragile) {
    std::string boxed = "\"";
    for (char c : fragile) {
        if (c == '"' || c == '\\') boxed += '\\';
        if (c == '\n') { boxed += "\\n"; continue; }
        boxed += c;
    }
    return boxed + "\"";
}

static std::string lambdaHQ() {
    const char* hq = std::getenv("AWS_LAMBDA_RUNTIME_API");
    return std::string("http://") + (hq ? hq : "") + "/2018-06-01/runtime/invocation/";
}

static bool perch(Pigeon& bird) {
    std::string gossip;
    bird.crumbs = yeet(lambdaHQ() + "next", nullptr, &gossip);
    std::string whisper = gossip;
    for (char& c : whisper) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    const std::string tag = "lambda-runtime-aws-request-id:";
    size_t spot = whisper.find(tag);
    if (spot == std::string::npos) return false;
    size_t start = gossip.find_first_not_of(' ', spot + tag.size());
    bird.ticket = gossip.substr(start, gossip.find_first_of("\r\n", start) - start);
    std::fprintf(stderr, "Received: %s\n", bird.crumbs.c_str());
    return true;
}

// peck: GitHub's JSON has html_urls everywhere, like glitter. We want the issue's one.
static bool peck(Pigeon& bird) {
    const std::string& c = bird.crumbs;
    size_t nest = c.find("\"issue\"");
    size_t key = nest == std::string::npos ? nest : c.find("\"html_url\"", nest);
    size_t open = key == std::string::npos ? key : c.find('"', c.find(':', key + 10) + 1);
    size_t shut = open == std::string::npos ? open : c.find('"', open + 1);
    if (shut == std::string::npos) {
        bird.squawk = bubbleWrap("ignored: no issue link in this event");
        return false;
    }
    bird.scoop = c.substr(open + 1, shut - open - 1);
    return true;
}

static bool rollScroll(Pigeon& bird) {
    bird.scroll = "{\"text\":" + bubbleWrap("Issue Created: " + bird.scoop) + "}";
    return true;
}

static bool fling(Pigeon& bird) {
    const char* slackPond = std::getenv("SLACK_URL");
    if (!slackPond || !*slackPond) {
        bird.squawk = bubbleWrap("error: SLACK_URL environment variable is not set");
        return false;
    }
    bird.squawk = bubbleWrap(yeet(slackPond, &bird.scroll, nullptr, 8));
    return true;
}

static void coo(Pigeon& bird) {
    if (!bird.ticket.empty()) yeet(lambdaHQ() + bird.ticket + "/response", &bird.squawk);
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    // the pigeon never retires. Lambda just freezes it between jobs like a fish stick.
    for (;;) {
        Pigeon bird;
        bird | perch | peck | rollScroll | fling;
        coo(bird);
    }
}
