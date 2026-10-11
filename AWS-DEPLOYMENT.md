# Deploying the Notice Board to AWS (EC2)

This walks through putting the app on a real, internet-reachable AWS server.
You'll need a free AWS account (aws.amazon.com) with the Free Tier — a
t3.micro/t2.micro instance costs $0 for the first 12 months under normal use.

I can't click through the AWS Console on your behalf — these steps need to
run in *your* AWS account. But if you paste back what you see at each step
(same as we did getting Node.js installed), I'll help you through any
errors live.

## Part A — Launch the server

1. **Sign in to the AWS Console** → search for **EC2** → click **Launch
   instance**.
2. **Name:** `notice-board-server` (anything you'll recognize).
3. **AMI (OS image):** choose **Ubuntu Server 22.04 LTS** (Free tier
   eligible, marked on the card).
4. **Instance type:** `t2.micro` or `t3.micro` (Free tier eligible).
5. **Key pair:** click **Create new key pair**, name it (e.g.
   `notice-board-key`), type RSA, format `.pem`. **Download it and keep it**
   — you cannot re-download it later, and you need it to connect.
6. **Network settings → Edit:** under inbound rules, allow:
   - SSH (port 22) — source: *My IP* (safer than Anywhere)
   - HTTP (port 80) — source: Anywhere
   - Custom TCP, port 3000 — source: Anywhere (so you can test before
     setting up Nginx on port 80)
7. Click **Launch instance**. Wait ~1 minute for it to start ("Running").
8. Copy its **Public IPv4 address** from the instance details page — you'll
   use this to connect and, later, to open the site in a browser.

## Part B — Connect to it

**Windows (PowerShell):**
```powershell
cd path\to\where\you\saved\the\.pem\file
icacls notice-board-key.pem /inheritance:r
icacls notice-board-key.pem /grant:r "%username%":R
ssh -i notice-board-key.pem ubuntu@YOUR_PUBLIC_IP
```

**Mac/Linux:**
```bash
cd path/to/where/you/saved/the/.pem/file
chmod 400 notice-board-key.pem
ssh -i notice-board-key.pem ubuntu@YOUR_PUBLIC_IP
```

Type `yes` if asked about the host fingerprint. You should land at an
`ubuntu@ip-...:~$` prompt — you're now inside the AWS server, not your own
laptop.

## Part C — Install Node.js on the server

```bash
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt-get install -y nodejs
node -v
```

## Part D — Get your project onto the server

Easiest path: re-zip your project (same `college-notice-board.zip` I gave
you, minus `node_modules`) and upload it. From **your own laptop's**
terminal (a new window — not the SSH session):

```bash
scp -i notice-board-key.pem college-notice-board.zip ubuntu@YOUR_PUBLIC_IP:~
```

Back in your **SSH session** (on the server):
```bash
sudo apt-get install -y unzip
unzip college-notice-board.zip
cd noticeboard
npm install
cp .env.example .env
nano .env   # change JWT_SECRET to something long and random; Ctrl+O, Enter, Ctrl+X to save
```

## Part E — Keep it running permanently

If you just run `npm start`, the server stops the moment you close the SSH
session. Use **pm2** instead, so it keeps running and restarts on crash or
reboot:

```bash
sudo npm install -g pm2
pm2 start server.js --name noticeboard
pm2 startup      # run the command it prints, starting with "sudo env..."
pm2 save
```

Now visit `http://YOUR_PUBLIC_IP:3000` in a browser — the board should load,
served from AWS instead of your laptop.

## Part F (optional, recommended) — Put it on port 80 with Nginx

So students don't need `:3000` in the URL:

```bash
sudo apt-get install -y nginx
sudo tee /etc/nginx/sites-available/noticeboard << 'EOF'
server {
    listen 80;
    server_name _;
    location / {
        proxy_pass http://localhost:3000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
EOF
sudo ln -s /etc/nginx/sites-available/noticeboard /etc/nginx/sites-enabled/
sudo rm -f /etc/nginx/sites-enabled/default
sudo nginx -t && sudo systemctl restart nginx
```

Now `http://YOUR_PUBLIC_IP` (no port) should work too.

## Part G (optional) — Docker instead of Parts C–E

If you'd rather use the included `Dockerfile`/`docker-compose.yml`:

```bash
sudo apt-get install -y docker.io docker-compose
sudo usermod -aG docker ubuntu   # log out and back in after this
cd noticeboard
cp .env.example .env
docker compose up -d --build
```

## Updating the site later

```bash
ssh -i notice-board-key.pem ubuntu@YOUR_PUBLIC_IP
cd noticeboard
# pull in your changed files (scp them over, as in Part D), then:
pm2 restart noticeboard
# — or, if using Docker:
docker compose up -d --build
```

## Shutting it down (so you're not billed)

Free Tier covers 750 instance-hours/month for 12 months — one instance
running continuously fits inside that. If you want to be safe, or you're
past the Free Tier window: **EC2 console → select the instance → Instance
state → Stop** (keeps it, no compute charges while stopped) or **Terminate**
(deletes it permanently).

## If something goes wrong

Paste me the exact command you ran and the exact output/error, the same way
we debugged the local setup — I'll walk through it with you.
