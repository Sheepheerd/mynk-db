from fastapi import FastAPI, UploadFile, File, HTTPException
from fastapi.responses import JSONResponse
from pydantic import BaseModel
from pathlib import Path
import hashlib
import json

app = FastAPI()

SERVER_DIR = Path("server_files")
SERVER_DIR.mkdir(exist_ok=True)
STATE_FILE = SERVER_DIR / ".mynk-state.json"

def hash_file(file_path: Path) -> str:
    """Return SHA256 hash of a file."""
    hasher = hashlib.sha256()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hasher.update(chunk)
    return hasher.hexdigest()

def load_state():
    """Load the state file, or rebuild if missing/corrupted."""
    if STATE_FILE.exists():
        try:
            with open(STATE_FILE, "r") as f:
                return json.load(f)
        except Exception:
            pass
    return rebuild_state()

def save_state(state):
    with open(STATE_FILE, "w") as f:
        json.dump(state, f, indent=2)

def rebuild_state():
    """Rebuild state by hashing all files in SERVER_DIR."""
    files = []
    for file_path in SERVER_DIR.glob("**/*"):
        if file_path.is_file() and file_path.name != STATE_FILE.name:
            files.append({
                "filename": str(file_path.relative_to(SERVER_DIR)),
                "hash": hash_file(file_path),
                "version": 0
            })
    state = {"files": files}
    save_state(state)
    return state

def update_state(filename: str, file_hash: str):
    """Update or add an entry for a file in the state."""
    state = load_state()
    files = state["files"]

    for entry in files:
        if entry["filename"] == filename:
            entry["hash"] = file_hash
            entry["version"] += 1
            break
    else:
        files.append({
            "filename": filename,
            "hash": file_hash,
            "version": 1
        })

    save_state(state)
    return state

class DeleteRequest(BaseModel):
    filename: str

@app.post("/upload")
async def upload_file(file: UploadFile = File(...)):
    file_path = SERVER_DIR / file.filename

    try:
        resolved_path = file_path.resolve()
        if SERVER_DIR.resolve() not in resolved_path.parents and resolved_path != SERVER_DIR.resolve():
            raise HTTPException(status_code=403, detail="Cannot upload file outside server directory")
    except Exception:
        raise HTTPException(status_code=400, detail="Invalid file path")

    file_path.parent.mkdir(parents=True, exist_ok=True)

    with open(file_path, "wb") as f:
        f.write(await file.read())

    file_hash = hash_file(file_path)
    state = update_state(str(file_path.relative_to(SERVER_DIR)), file_hash)

    return {"filename": str(file_path.relative_to(SERVER_DIR)), "hash": file_hash, "state": state}

@app.delete("/delete")
async def delete_file(req: DeleteRequest):
    file_path = SERVER_DIR / req.filename
    if not file_path.exists():
        raise HTTPException(status_code=404, detail="File not found")
    file_path.unlink()

    state = load_state()
    state["files"] = [f for f in state["files"] if f["filename"] != req.filename]
    save_state(state)

    return {"status": "deleted", "filename": req.filename, "state": state}

@app.get("/structure")
async def get_structure():
    state = load_state()
    return state

@app.get("/file/{file_path:path}")
async def get_file_contents(file_path: str):
    file_path_obj = SERVER_DIR / file_path
    try:
        resolved_path = file_path_obj.resolve()
        if SERVER_DIR.resolve() not in resolved_path.parents and resolved_path != SERVER_DIR.resolve():
            raise HTTPException(status_code=403, detail="Access to file outside server directory is forbidden")
    except FileNotFoundError:
        raise HTTPException(status_code=404, detail="File not found")

    if not file_path_obj.exists() or not file_path_obj.is_file():
        raise HTTPException(status_code=404, detail="File not found")

    try:
        with open(file_path_obj, "r", encoding="utf-8") as f:
            contents = f.read()
        return {"filename": file_path, "contents": contents}
    except UnicodeDecodeError:
        raise HTTPException(status_code=400, detail="File is not a text file or cannot be decoded as UTF-8")
