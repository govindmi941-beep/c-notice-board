# This Project, Explained Through Cloud Computing Concepts

Use this as background for your report or viva — it maps the Notice Board's
actual design to the standard concepts your Cloud Computing syllabus covers.

## 1. Why this is a cloud computing project, not just a website

A plain website is any HTML page a server hands out. What makes this a
**cloud computing** project is the architecture underneath it, built so it
*can* run on shared, remotely-managed infrastructure rather than one
physical machine you own:

- **Three-tier architecture** — presentation (the browser pages), application
  (the Express API in `server.js`), and data (`db.js` + the data store) are
  separated. Each tier can be deployed, scaled, or replaced independently —
  exactly how cloud-hosted applications are structured.
- **Statelessness** — the API issues JWTs instead of server-side sessions.
  No user's data lives "in" one server's memory, so any server instance can
  answer any request. This is what makes horizontal scaling possible in the
  cloud (see §3).
- **Environment-based configuration** (`.env` / `JWT_SECRET`, `PORT`, admin
  credentials) — the same code runs unmodified on your laptop, in a Docker
  container, or on an AWS EC2 instance. Cloud platforms expect config to
  come from the environment, not hardcoded files.

## 2. NIST's five essential characteristics, applied here

| Characteristic | What it means | How it applies to this project |
|---|---|---|
| **On-demand self-service** | Provision compute without human intervention from the provider | Launching an EC2 instance or an Elastic Beanstalk environment to host this app takes minutes, via console or CLI — no ticket to IT |
| **Broad network access** | Reachable over the network from standard clients | The frontend is plain HTTP/HTML/JS — works from any browser, laptop or phone, once deployed |
| **Resource pooling** | Provider's infrastructure is shared across many tenants, with location transparency | Your EC2 instance runs on shared AWS hardware; you don't know or care which physical server it's on |
| **Rapid elasticity** | Capacity can scale out/in quickly to match demand | During exam-notice season (high traffic) you could run 2–3 instances behind a load balancer; scale back to 1 afterward |
| **Measured service** | Usage is metered; you pay for what you use | AWS bills by EC2 instance-hour, data transfer, and storage — not a flat fee |

## 3. Service model: where does this app sit?

- **IaaS (Infrastructure as a Service)** — if you deploy on a raw EC2
  instance and manage the OS, Node runtime, and process yourself (the
  `AWS-DEPLOYMENT.md` guide in this project does exactly this). You're
  renting a virtual machine; everything above the OS is your job.
- **PaaS (Platform as a Service)** — if you instead deploy via **AWS Elastic
  Beanstalk** (mentioned as an alternative in the deployment guide), AWS
  manages the OS, patching, and load balancer for you — you just hand over
  code.
- **SaaS (Software as a Service)** — from a *student's* point of view once
  it's deployed, the Notice Board itself is consumed as SaaS: they just open
  a URL in a browser, no installation, no infrastructure knowledge needed.

Worth saying explicitly in your viva: **the same codebase demonstrates
all three layers depending on how it's deployed** — that's a good point to
make if asked to compare IaaS/PaaS/SaaS.

## 4. Deployment model

This project uses (or targets) the **public cloud** (AWS) — appropriate
because:
- The college doesn't need to own/maintain physical servers for a small
  departmental tool.
- Public cloud gives pay-as-you-go pricing, which suits a low-traffic
  internal tool far better than buying dedicated hardware.
- A **private cloud** (college-owned data center) or **hybrid** model would
  only make sense at much larger scale (university-wide ERP, sensitive data
  regulations) — worth mentioning as a contrast if asked "why not private
  cloud?"

## 5. Mapping components to AWS services

| This project's component | Local/current | Equivalent AWS service |
|---|---|---|
| Static frontend (`public/`) | served by Express | **S3 + CloudFront** (optional split) or served from the same EC2 instance |
| Backend API (`server.js`) | Node process on your laptop | **EC2** (IaaS) or **Elastic Beanstalk** (PaaS) |
| Data storage (`db.js`) | local JSON file | **RDS** (managed Postgres/MySQL) for real multi-instance scalability, or **DynamoDB** if you want to stay NoSQL |
| Authentication (JWT + bcrypt) | self-contained in the app | unchanged — or **Cognito** if you wanted AWS-managed auth |
| Secrets (`JWT_SECRET`, admin password) | `.env` file | **AWS Systems Manager Parameter Store** or **Secrets Manager** |
| Traffic entry point | `localhost:3000` | **Application Load Balancer** (distributes across multiple EC2 instances) |
| DNS | `localhost` | **Route 53** (e.g. `notices.yourcollege.edu`) |
| Monitoring | terminal logs | **CloudWatch** (logs, metrics, alarms) |
| Access control | OS user on your laptop | **IAM** roles/policies, **Security Groups** as a virtual firewall |

## 6. An honest limitation worth naming in your viva

The current data store (`noticeboard-data.json`, a single file on disk) is
**not** naturally horizontally scalable — if you ran two EC2 instances
behind a load balancer right now, each would have its own separate file,
so a notice posted via instance A wouldn't show on instance B. This is a
real and common cloud-architecture tradeoff (local file storage vs. a
shared, managed database), and naming it shows you understand *why* a
managed database like RDS exists in the first place: it gives every
instance a single, shared, consistent source of truth. The README and this
document both flag RDS as the natural next step for exactly this reason.

## 7. Elasticity & cost example, for a report paragraph

> "The Notice Board is deployed on a single t3.micro EC2 instance
> (AWS Free Tier eligible) during normal operation. During peak periods —
> such as results day or admission deadlines, when hundreds of students may
> check the board simultaneously — an Auto Scaling Group could launch
> additional instances behind an Application Load Balancer, then
> automatically terminate them once traffic subsides. Because the backend
> is stateless (JWT-based auth, no server-side sessions), any instance can
> serve any request, which is what makes this kind of elastic scaling
> possible without extra engineering."

Feel free to paraphrase that directly into your report.
