from abc import ABC
from typing import TypeVar, Type, Any

ConfigurationType = TypeVar('ConfigurationType')


class ConfigurationInterface(ABC):
    def get_configuration(self,
                          config_name: str,
                          config_type: Type[ConfigurationType],
                          default: Any = None) -> ConfigurationType:
        raise NotImplementedError
