# Put the C Notice Board online (no AWS, no credit card)

Render is a cloud platform that builds your project from GitHub and gives
it a public https:// link. In cloud terms this is **PaaS** (Platform as a
Service): you give it code, it manages the server, OS and networking.
(Compare with AWS EC2, which is **IaaS**: you manage the server yourself.)

## Steps

1. **Put the project on GitHub.** Create a free account at github.com,
   click New repository (name: `c-notice-board`), then upload every file
   from the `c-noticeboard` folder using "uploading an existing file".
   The `Dockerfile` must be at the top level of the repository.
2. **Create a Render account** at render.com (sign up with GitHub).
3. Click **New +** then **Web Service**, and pick your `c-notice-board`
   repository.
4. Set **Language/Runtime** to **Docker**, and **Instance type** to **Free**.
5. Under **Environment Variables** add `ADMIN_PASS` with a password of
   your own choice (this replaces the default `admin123`).
6. Click **Create Web Service**. Wait for the build log to finish
   (a few minutes). When it says the service is live, open the
   `https://your-name.onrender.com` link at the top of the page.
   Your admin page is at `/admin`.

## What to know before the viva

- The free service **sleeps after about 15 minutes of no visitors**, so the
  first page load afterwards can take about a minute. Open the link a few
  minutes before you present.
- Free services have **temporary storage**: when the service restarts or
  redeploys, `notices.dat` resets to the 3 sample notices. That is fine for
  a demo, but real systems use a managed database (e.g. RDS) for this reason.
- The admin password travels in the URL (`?key=...`). Acceptable for a
  classroom demo; mention it as a known limitation.
- Free-tier terms change, so check the plan details on render.com while
  signing up.
