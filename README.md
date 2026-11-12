# esep-webhooks 🐦

ESEP Assignment 8: open an issue here and a C++ carrier pigeon flies the link to Slack.

```
GitHub issue -> API Gateway (POST /default) -> Lambda (relay.cpp) -> Slack
```

The pigeon runs as a Lambda custom runtime (`provided.al2023`, arm64). Each job it:

| Move | What it does |
|---|---|
| `perch` | waits for Lambda to hand over the next event |
| `peck` | digs `issue.html_url` out of GitHub's JSON |
| `rollScroll` | wraps it as `{"text": "Issue Created: <link>"}` |
| `fling` | POSTs it to the URL in the `SLACK_URL` environment variable |
| `coo` | reports back to Lambda (always, even if a move gave up) |

No issue in the event (like GitHub's `ping`)? The pigeon shrugs, says so, and does not crash.

## Build

Needs Docker. `bash build.sh` makes `function.zip`, ready for Lambda.

The Slack URL lives only in the Lambda's environment variables, never in this repo.
