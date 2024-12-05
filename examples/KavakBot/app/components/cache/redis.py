import redis
from examples.KavakBot.app.components.cache.cache_interface import CacheInterface


class AsyncRedis(CacheInterface):

    def __init__(self, host='localhost', port=6379, db=0):
        self.client = redis.StrictRedis(host=host, port=port, db=db)

    async def expire(self, key: str, seconds: int) -> None:
        await self.client.expire(key, seconds)

    async def set(self, key: str, value: str) -> None:
        await self.client.set(key, value)

    async def get(self, key: str) -> str:
        value: bytes = await self.client.get(key)
        return value.decode('utf-8') if value else None

    async def delete(self, key: str) -> None:
        await self.client.delete(key)

    async def exists(self, key: str) -> bool:
        return await self.client.exists(key) == 1
