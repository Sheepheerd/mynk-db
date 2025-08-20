from fastapi import APIRouter, Path
from pydantic import BaseModel

router = APIRouter(
    prefix="/sync",
    tags=["sync"],
    dependencies=[],
    responses={404: {"description": "Not found"}},
)

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

# @router.get("/sync/{username}", tags=["examples"])
# async def read_user(username: str):
#     return {"username": username}

@router.post("/sync")
async def sync_files(sync_request: SyncRequest):
    root_dir = Path("server_files")
    root_dir.mkdir(exist_ok=True)

    client_summary = {entry.filename: entry.hash for entry in sync_request.summary}

    response_files = []

    all_filenames = set(client_summary.keys()) | set(server_files.keys())

