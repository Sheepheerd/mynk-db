from fastapi import FastAPI
from pydantic import BaseModel
from pathlib import Path

app = FastAPI()

class FileEntry(BaseModel):
    filename: str
    version: int
    contents: str
    hash: str
    action: str 

class SummaryEntry(BaseModel):
    filename: str
    hash: str

class SyncRequest(BaseModel):
    files: list[FileEntry]
    summary: list[SummaryEntry]

class ResponseFileEntry(BaseModel):
    filename: str
    contents: str | None 

@app.post("/sync")
async def sync_files(sync_request: SyncRequest):
    root_dir = Path("server_files")
    root_dir.mkdir(exist_ok=True)

    client_summary = {entry.filename: entry.hash for entry in sync_request.summary}
    server_files = {
        str(path.relative_to(root_dir)): hash_file(path)
        for path in root_dir.rglob("*")
        if path.is_file()
    }

    response_files = []

    all_filenames = set(client_summary.keys()) | set(server_files.keys())

    # for filename in all_filenames:

    # return response_files

def hash_file(file_path: Path) -> str:
    import hashlib

    sha256 = hashlib.sha256()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            sha256.update(chunk)
    return sha256.hexdigest();turn ;{"item_id": item_id, "q": q}
