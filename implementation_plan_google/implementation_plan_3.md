# Implementation Plan: System Mail Configuration & Multi-Tenant Architecture

Clean up the public user login modal (`EmailAuthModal.qml`), move all mail server credentials management to the **Admin Dashboard**, and build the Multi-Tenant database structure for total Admin control.

## User Review Required

> [!IMPORTANT]
> - **Public Login Modal**: The "Owner Mail Credentials" box is completely removed from `EmailAuthModal.qml`. Regular users logging in will only see a clean, professional Gmail sign-in & 6-digit OTP verification interface.
> - **Admin Dashboard**: Mail Server Credentials (Sender Email & API Key) are now managed exclusively inside the **Admin Dashboard** ([AdminDashboard.qml](file:///file:///Users/maitraprajapati/Desktop/ssA31/src/ui/components/AdminDashboard.qml)).
> - **Multi-Tenant Foundation**: SQLite database schema (`users` & `tenants`) is updated to track `tenant_id`, `tenant_name`, and tenant user limits, placing full control of all tenants and sub-users directly in the Admin's hands.

---

## Proposed Changes

### UI & Public Login

#### [MODIFY] [EmailAuthModal.qml](file:///Users/maitraprajapati/Desktop/ssA31/src/ui/components/EmailAuthModal.qml)
- Remove the "Owner Mail Credentials" card completely from the user login screen.
- Streamline height and visuals to focus strictly on Gmail address input, password entry, and OTP code verification.

#### [MODIFY] [UserProfileBadge.qml](file:///Users/maitraprajapati/Desktop/ssA31/src/ui/components/UserProfileBadge.qml)
- Remove the mail config inputs from the regular user dropdown menu.
- Keep only standard user profile actions (custom display username, plan stats, logout).

---

### Admin Dashboard & Management

#### [MODIFY] [AdminDashboard.qml](file:///Users/maitraprajapati/Desktop/ssA31/src/ui/components/AdminDashboard.qml)
- Add **System Mail Configuration Card** (Sender Email & API Key) at the top of the Admin Dashboard.
- Add **Tenant Overview & Controls** section to view tenant allocations, assigned sub-users, and tenant capacity.

#### [MODIFY] [UserManager.h](file:///Users/maitraprajapati/Desktop/ssA31/src/auth/UserManager.h) & [UserManager.cpp](file:///Users/maitraprajapati/Desktop/ssA31/src/auth/UserManager.cpp)
- Extend SQLite database schema with `tenant_id` and `tenant_name` columns for users.
- Add helper methods to register/assign users to specific tenants and query tenant user counts.

---

## Verification Plan

### Automated Tests
- Build application with `cmake --build build`.
- Verify 0 compilation errors.

### Manual Verification
- Open public login window (`EmailAuthModal`): Confirm no mail credential fields are visible.
- Log in as Admin (`admin` / `admin`) -> Open Admin Dashboard.
- Verify System Mail Configuration card is visible and functional in Admin Dashboard.
- Verify Tenant & User management list displays tenant assignments properly.
