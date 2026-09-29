# Implementation Plan: Per-Tenant & Solo User Custom Export Limits

Integrate an Admin configuration engine to set custom video export limits (Daily Export Count, Max Video Duration, Max Resolution) individually for **each Tenant**, and a unified global export policy for **Solo Users**.

## User Review Required

> [!IMPORTANT]
> - **Per-Tenant Custom Limits**: Admin can configure unique export rules for each corporate tenant (`Acme Corp`, `TechLabs Inc`, etc.):
>   - **Daily Exports Count** (e.g. 5, 10, 20, or Unlimited)
>   - **Max Export Duration** (e.g. 2 min, 5 min, 15 min, 30 min, Unlimited)
>   - **Max Resolution** (1080p, 1440p, or 4K)
> - **Solo Users Global Policy**: All solo users share a global policy configured by Admin in the Admin Dashboard.
> - **Quota Engine (`UserQuotaManager`)**: Automatically queries and enforces the specific tenant's or solo user's policy when exporting videos.

### Answer
for this part i want to have custom number for daily export count and custom number of max export duration 

---

## Proposed Changes

### Database & Backend (`UserManager` & `UserQuotaManager`)

#### [MODIFY] [UserManager.h](file:///Users/maitraprajapati/Desktop/ssA31/src/auth/UserManager.h) & [UserManager.cpp](file:///Users/maitraprajapati/Desktop/ssA31/src/auth/UserManager.cpp)
- Add `tenants` SQLite database table:
  - `tenant_id` TEXT PRIMARY KEY
  - `daily_export_limit` INTEGER (default: 10, -1 for Unlimited)
  - `max_duration_mins` INTEGER (default: 10, -1 for Unlimited)
  - `max_resolution_height` INTEGER (default: 2160, e.g., 1080/1440/2160)
- Add `getTenantLimits(tenantId)` and `setTenantLimits(tenantId, dailyLimit, maxDurationMins, maxResHeight)` C++ methods.
- Add `getSoloPolicy()` and `setSoloPolicy(dailyLimit, maxDurationMins, maxResHeight)` methods.

#### [MODIFY] [UserQuotaManager.h](file:///Users/maitraprajapati/Desktop/ssA31/src/auth/UserQuotaManager.h) & [UserQuotaManager.cpp](file:///Users/maitraprajapati/Desktop/ssA31/src/auth/UserQuotaManager.cpp)
- Update `dailyExportLimit()`, `maxDurationMs()`, and `maxResolutionHeight()` to query the active user's specific tenant policy if assigned to a tenant, or the solo policy if a solo user.

---

### Admin Dashboard UI (`AdminDashboard.qml`)

#### [MODIFY] [AdminDashboard.qml](file:///Users/maitraprajapati/Desktop/ssA31/src/ui/components/AdminDashboard.qml)
- **Tenant Card Controls**: Add a **"⚙ Configure Limits"** button on each tenant card.
- **Tenant Limits Config Modal**: Opens a dialog to edit the tenant's daily export limit, max duration, and max resolution.
- **Solo Policy Config Box**: Add a **"⚙ Global Solo User Policy Settings"** card in the Solo Users tab.

---

## Verification Plan

### Automated Tests
- Build application with `cmake --build build` (0 errors).

### Manual Verification
- Log in as Admin -> Open Admin Dashboard.
- Click **"⚙ Configure Limits"** on `Acme Corp` tenant card:
  - Change `Acme Corp` limits to: 15 exports/day, 15 min duration, 4K resolution.
  - Save limits and verify persistence in database.
- Click **"⚙ Global Solo Policy Settings"**:
  - Change Solo limits to: 3 exports/day, 3 min duration, 1080p.
  - Save and verify enforcement in quota manager.
